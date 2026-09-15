/*
 * Heltec LoRa 32 - Unified Cistern Monitor
 * Auto-detects transmitter vs receiver mode based on sensor presence
 * 
 * Mode Detection:
 * - If ultrasonic sensor detected on GPIO 13/12 -> TRANSMITTER MODE
 * - If no sensor detected -> RECEIVER MODE
 * 
 * Manual Override:
 * - Hold PRG button during boot to force mode selection
 * 
 * Libraries needed:
 * - Heltec ESP32 Dev-Boards
 * - WiFiManager by tzapu
 * - PubSubClient (MQTT)
 * - ArduinoJson
 */

#include <heltec.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <Preferences.h>
#include <ArduinoJson.h>

// Pin definitions
#define TRIG_PIN 13
#define ECHO_PIN 12
#define BUTTON_PIN 0  // PRG button on Heltec board

// LoRa Configuration
#define BAND 915E6  // Change to 868E6 for Europe, 433E6 for Asia
#define SYNC_WORD 0x42

// Mode enumeration
enum DeviceMode {
  MODE_UNKNOWN = 0,
  MODE_TRANSMITTER = 1,
  MODE_RECEIVER = 2
};

// Device mode (will be auto-detected)
DeviceMode deviceMode = MODE_UNKNOWN;

// Preferences storage
Preferences preferences;

// Configuration structure
struct Config {
  char deviceName[32];
  char deviceType[16];  // "transmitter" or "receiver"
  
  // Transmitter settings
  float cisternHeight;
  float sensorOffset;
  int measureInterval;
  
  // Receiver settings
  char mqttServer[64];
  int mqttPort;
  char mqttUser[32];
  char mqttPass[32];
  char mqttTopic[64];
  char haDiscoveryPrefix[32];
  
  // Common settings
  unsigned long loraFreq;
  char loraKey[33];
} config;

// MQTT Client (receiver only)
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Transmitter data
struct TransmitterData {
  float waterLevel;
  float waterPercent;
  float distance;
  unsigned long lastMeasurement;
  int measurementCount;
} txData;

// Receiver data
struct ReceiverData {
  String deviceName;
  float distance;
  float level;
  float percent;
  float battery;
  int count;
  int rssi;
  unsigned long lastUpdate;
  bool dataValid;
} rxData;

// Connection state
String displayStatus = "Initializing...";
unsigned long lastMqttAttempt = 0;
unsigned long lastDisplayUpdate = 0;
bool wifiConnected = false;
bool mqttConnected = false;

void setup() {
  Serial.begin(115200);
  delay(100);
  
  Serial.println("\n\n=================================");
  Serial.println("Cistern Monitor - Unified Firmware");
  Serial.println("=================================\n");
  
  // Initialize Heltec board with temporary frequency
  Heltec.begin(true /*DisplayEnable*/, true /*LoRa Enable*/, true /*Serial Enable*/, 
               true /*PABOOST*/, BAND);
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  // Show startup screen
  displayInfo("Cistern Monitor", "Detecting mode...", "v1.0");
  delay(2000);
  
  // Check if button held for manual mode selection
  if (digitalRead(BUTTON_PIN) == LOW) {
    Serial.println("Button held - entering manual mode selection...");
    manualModeSelection();
  } else {
    // Auto-detect mode
    autoDetectMode();
  }
  
  // Load configuration
  loadConfig();
  
  // Configure LoRa with correct frequency from config
  LoRa.setFrequency(config.loraFreq);
  
  // Initialize based on mode
  if (deviceMode == MODE_TRANSMITTER) {
    setupTransmitter();
  } else if (deviceMode == MODE_RECEIVER) {
    setupReceiver();
  } else {
    Serial.println("ERROR: Mode detection failed!");
    displayInfo("ERROR", "Mode Detection", "Failed!");
    while(1) delay(1000);
  }
  
  displayStatus = "Ready";
  Serial.println("\nDevice ready!\n");
}

void loop() {
  // Check for config mode trigger (hold button 3 seconds)
  static unsigned long buttonPressStart = 0;
  static bool buttonWasPressed = false;
  
  if (digitalRead(BUTTON_PIN) == LOW && !buttonWasPressed) {
    buttonWasPressed = true;
    buttonPressStart = millis();
  } else if (digitalRead(BUTTON_PIN) == HIGH && buttonWasPressed) {
    buttonWasPressed = false;
  } else if (buttonWasPressed && (millis() - buttonPressStart) >= 3000) {
    enterConfigMode();
    buttonWasPressed = false;
  }
  
  // Run appropriate loop based on mode
  if (deviceMode == MODE_TRANSMITTER) {
    loopTransmitter();
  } else if (deviceMode == MODE_RECEIVER) {
    loopReceiver();
  }
  
  delay(10);
}

// ============================================
// MODE DETECTION
// ============================================

void autoDetectMode() {
  Serial.println("Starting automatic mode detection...");
  displayInfo("Auto Detect", "Testing sensor...", "");
  
  // Try to detect ultrasonic sensor
  bool sensorDetected = testUltrasonicSensor();
  
  if (sensorDetected) {
    deviceMode = MODE_TRANSMITTER;
    Serial.println("✓ Ultrasonic sensor detected -> TRANSMITTER MODE");
    displayInfo("Mode Detected", "TRANSMITTER", "Sensor found");
  } else {
    deviceMode = MODE_RECEIVER;
    Serial.println("✗ No sensor detected -> RECEIVER MODE");
    displayInfo("Mode Detected", "RECEIVER", "No sensor");
  }
  
  delay(2000);
}

bool testUltrasonicSensor() {
  // Send test pulses and check for response
  int validReadings = 0;
  
  for (int i = 0; i < 3; i++) {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    
    long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout
    
    if (duration > 0 && duration < 25000) { // Valid range
      validReadings++;
    }
    delay(100);
  }
  
  // If we get at least 2 valid readings, sensor is present
  return (validReadings >= 2);
}

void manualModeSelection() {
  displayInfo("Manual Select", "1. Transmitter", "2. Receiver");
  Serial.println("\n=== MANUAL MODE SELECTION ===");
  Serial.println("Choose mode:");
  Serial.println("1. Transmitter (with sensor)");
  Serial.println("2. Receiver (WiFi/MQTT)");
  Serial.println("\nWaiting 10 seconds for button press...");
  
  unsigned long startTime = millis();
  int selection = 0;
  
  // Wait for button press or timeout
  while (millis() - startTime < 10000 && selection == 0) {
    if (digitalRead(BUTTON_PIN) == HIGH) {
      // Button released - short press
      delay(500);
      if (digitalRead(BUTTON_PIN) == LOW) {
        // Button pressed again
        selection = 1; // Toggle through options
        displayInfo("Manual Select", "-> Receiver", "Press to confirm");
        Serial.println("Option: Receiver");
        delay(1000);
        
        // Wait for release
        while (digitalRead(BUTTON_PIN) == LOW) delay(10);
        delay(500);
        
        // Wait for confirmation or timeout
        unsigned long confirmStart = millis();
        while (millis() - confirmStart < 5000) {
          if (digitalRead(BUTTON_PIN) == LOW) {
            selection = 2;
            break;
          }
        }
      }
    }
  }
  
  if (selection == 2) {
    deviceMode = MODE_RECEIVER;
    Serial.println("✓ User selected: RECEIVER");
    displayInfo("Selected", "RECEIVER MODE", "");
  } else {
    deviceMode = MODE_TRANSMITTER;
    Serial.println("✓ User selected: TRANSMITTER (default)");
    displayInfo("Selected", "TRANSMITTER MODE", "");
  }
  
  delay(2000);
}

// ============================================
// CONFIGURATION
// ============================================

void loadConfig() {
  preferences.begin("cistern", false);
  
  // Determine if we have saved mode
  String savedType = preferences.getString("deviceType", "");
  
  if (savedType == "") {
    // First time setup - save detected mode
    if (deviceMode == MODE_TRANSMITTER) {
      strcpy(config.deviceType, "transmitter");
    } else {
      strcpy(config.deviceType, "receiver");
    }
    preferences.putString("deviceType", config.deviceType);
  } else {
    strcpy(config.deviceType, savedType.c_str());
    // Verify saved mode matches detection
    if ((strcmp(config.deviceType, "transmitter") == 0 && deviceMode == MODE_RECEIVER) ||
        (strcmp(config.deviceType, "receiver") == 0 && deviceMode == MODE_TRANSMITTER)) {
      Serial.println("WARNING: Saved mode doesn't match detection!");
      Serial.printf("Saved: %s, Detected: %s\n", config.deviceType, 
                    deviceMode == MODE_TRANSMITTER ? "transmitter" : "receiver");
    }
  }
  
  // Common settings
  String name = preferences.getString("deviceName", 
                deviceMode == MODE_TRANSMITTER ? "Cistern-TX" : "Cistern-RX");
  name.toCharArray(config.deviceName, 32);
  
  config.loraFreq = preferences.getULong("loraFreq", BAND);
  String key = preferences.getString("loraKey", "00112233445566778899AABBCCDDEEFF");
  key.toCharArray(config.loraKey, 33);
  
  // Transmitter settings
  config.cisternHeight = preferences.getFloat("cisternHeight", 200.0);
  config.sensorOffset = preferences.getFloat("sensorOffset", 10.0);
  config.measureInterval = preferences.getInt("measureInterval", 60);
  
  // Receiver settings
  String mqtt = preferences.getString("mqttServer", "homeassistant.local");
  mqtt.toCharArray(config.mqttServer, 64);
  config.mqttPort = preferences.getInt("mqttPort", 1883);
  String user = preferences.getString("mqttUser", "");
  user.toCharArray(config.mqttUser, 32);
  String pass = preferences.getString("mqttPass", "");
  pass.toCharArray(config.mqttPass, 32);
  String topic = preferences.getString("mqttTopic", "homeassistant/sensor/cistern");
  topic.toCharArray(config.mqttTopic, 64);
  String discovery = preferences.getString("haDiscovery", "homeassistant");
  discovery.toCharArray(config.haDiscoveryPrefix, 32);
  
  preferences.end();
  
  Serial.println("\nConfiguration loaded:");
  Serial.printf("  Mode: %s\n", config.deviceType);
  Serial.printf("  Device: %s\n", config.deviceName);
  if (deviceMode == MODE_TRANSMITTER) {
    Serial.printf("  Cistern Height: %.1f cm\n", config.cisternHeight);
    Serial.printf("  Interval: %d sec\n", config.measureInterval);
  } else {
    Serial.printf("  MQTT: %s:%d\n", config.mqttServer, config.mqttPort);
  }
}

void saveConfig() {
  preferences.begin("cistern", false);
  preferences.putString("deviceType", config.deviceType);
  preferences.putString("deviceName", config.deviceName);
  preferences.putULong("loraFreq", config.loraFreq);
  preferences.putString("loraKey", config.loraKey);
  
  if (deviceMode == MODE_TRANSMITTER) {
    preferences.putFloat("cisternHeight", config.cisternHeight);
    preferences.putFloat("sensorOffset", config.sensorOffset);
    preferences.putInt("measureInterval", config.measureInterval);
  } else {
    preferences.putString("mqttServer", config.mqttServer);
    preferences.putInt("mqttPort", config.mqttPort);
    preferences.putString("mqttUser", config.mqttUser);
    preferences.putString("mqttPass", config.mqttPass);
    preferences.putString("mqttTopic", config.mqttTopic);
    preferences.putString("haDiscovery", config.haDiscoveryPrefix);
  }
  
  preferences.end();
  Serial.println("Configuration saved!");
}

void enterConfigMode() {
  Serial.println("Entering configuration mode...");
  
  String portalName = "Cistern_" + String(config.deviceType);
  displayInfo("Config Mode", "WiFi:", portalName);
  
  WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(300);
  
  // Common parameters
  WiFiManagerParameter custom_device_name("device", "Device Name", config.deviceName, 32);
  WiFiManagerParameter custom_lora_key("key", "LoRa Key (32 hex)", config.loraKey, 33);
  
  wifiManager.addParameter(&custom_device_name);
  
  // Mode-specific parameters
  WiFiManagerParameter *custom_params[10];
  int paramCount = 0;
  
  if (deviceMode == MODE_TRANSMITTER) {
    custom_params[paramCount++] = new WiFiManagerParameter("sep1", "<br/><b>Transmitter Settings</b>");
    custom_params[paramCount++] = new WiFiManagerParameter("height", "Cistern Height (cm)", 
                                                           String(config.cisternHeight).c_str(), 10);
    custom_params[paramCount++] = new WiFiManagerParameter("offset", "Sensor Offset (cm)", 
                                                           String(config.sensorOffset).c_str(), 10);
    custom_params[paramCount++] = new WiFiManagerParameter("interval", "Measure Interval (sec)", 
                                                           String(config.measureInterval).c_str(), 10);
  } else {
    custom_params[paramCount++] = new WiFiManagerParameter("sep1", "<br/><b>Receiver Settings</b>");
    custom_params[paramCount++] = new WiFiManagerParameter("mqtt", "MQTT Server", config.mqttServer, 64);
    custom_params[paramCount++] = new WiFiManagerParameter("port", "MQTT Port", 
                                                           String(config.mqttPort).c_str(), 6);
    custom_params[paramCount++] = new WiFiManagerParameter("user", "MQTT User", config.mqttUser, 32);
    custom_params[paramCount++] = new WiFiManagerParameter("pass", "MQTT Password", config.mqttPass, 32);
    custom_params[paramCount++] = new WiFiManagerParameter("topic", "MQTT Topic", config.mqttTopic, 64);
    custom_params[paramCount++] = new WiFiManagerParameter("ha", "HA Discovery", 
                                                           config.haDiscoveryPrefix, 32);
  }
  
  wifiManager.addParameter(&custom_lora_key);
  for (int i = 0; i < paramCount; i++) {
    wifiManager.addParameter(custom_params[i]);
  }
  
  if (!wifiManager.startConfigPortal(portalName.c_str())) {
    Serial.println("Failed to connect or timeout");
    displayInfo("Config Failed", "Restarting...", "");
    delay(3000);
    ESP.restart();
  }
  
  // Save configuration
  strcpy(config.deviceName, custom_device_name.getValue());
  strcpy(config.loraKey, custom_lora_key.getValue());
  
  if (deviceMode == MODE_TRANSMITTER) {
    config.cisternHeight = atof(custom_params[1]->getValue());
    config.sensorOffset = atof(custom_params[2]->getValue());
    config.measureInterval = atoi(custom_params[3]->getValue());
  } else {
    strcpy(config.mqttServer, custom_params[1]->getValue());
    config.mqttPort = atoi(custom_params[2]->getValue());
    strcpy(config.mqttUser, custom_params[3]->getValue());
    strcpy(config.mqttPass, custom_params[4]->getValue());
    strcpy(config.mqttTopic, custom_params[5]->getValue());
    strcpy(config.haDiscoveryPrefix, custom_params[6]->getValue());
  }
  
  saveConfig();
  
  // Cleanup
  for (int i = 0; i < paramCount; i++) {
    delete custom_params[i];
  }
  
  displayInfo("Config Saved", "Restarting...", "");
  delay(2000);
  ESP.restart();
}

// ============================================
// TRANSMITTER MODE
// ============================================

void setupTransmitter() {
  Serial.println("\n=== TRANSMITTER MODE ===");
  
  txData.waterLevel = 0;
  txData.waterPercent = 0;
  txData.distance = 0;
  txData.lastMeasurement = 0;
  txData.measurementCount = 0;
  
  setupLoRa();
  
  displayInfo("Transmitter", config.deviceName, "Ready");
  delay(1000);
}

void loopTransmitter() {
  // Take measurement at interval
  if (millis() - txData.lastMeasurement >= (config.measureInterval * 1000)) {
    takeMeasurement();
    transmitData();
    txData.lastMeasurement = millis();
    txData.measurementCount++;
  }
  
  // Update display
  if (millis() - lastDisplayUpdate > 500) {
    updateDisplayTransmitter();
    lastDisplayUpdate = millis();
  }
}

void takeMeasurement() {
  float totalDistance = 0;
  int validReadings = 0;
  
  for (int i = 0; i < 5; i++) {
    float reading = measureDistance();
    if (reading > 0 && reading < 400) {
      totalDistance += reading;
      validReadings++;
    }
    delay(50);
  }
  
  if (validReadings > 0) {
    txData.distance = totalDistance / validReadings;
    txData.waterLevel = config.cisternHeight - txData.distance - config.sensorOffset;
    
    if (txData.waterLevel < 0) txData.waterLevel = 0;
    if (txData.waterLevel > config.cisternHeight) txData.waterLevel = config.cisternHeight;
    
    txData.waterPercent = (txData.waterLevel / config.cisternHeight) * 100.0;
    displayStatus = "Measured";
    
    Serial.printf("Distance: %.1f cm, Level: %.1f cm (%.1f%%)\n", 
                  txData.distance, txData.waterLevel, txData.waterPercent);
  } else {
    displayStatus = "Read Error";
    Serial.println("Failed to get valid reading");
  }
}

float measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return -1;
  
  return duration * 0.0343 / 2.0;
}

void transmitData() {
  StaticJsonDocument<200> doc;
  doc["device"] = config.deviceName;
  doc["distance"] = round(txData.distance * 10) / 10.0;
  doc["level"] = round(txData.waterLevel * 10) / 10.0;
  doc["percent"] = round(txData.waterPercent * 10) / 10.0;
  doc["battery"] = getBatteryVoltage();
  doc["count"] = txData.measurementCount;
  
  String payload;
  serializeJson(doc, payload);
  
  String encrypted = xorEncrypt(payload, config.loraKey);
  
  LoRa.beginPacket();
  LoRa.print(encrypted);
  LoRa.endPacket();
  
  displayStatus = "Transmitted";
  Serial.println("Transmitted: " + payload);
}

void updateDisplayTransmitter() {
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_10);
  
  Heltec.display->drawString(0, 0, "TX: " + String(config.deviceName));
  Heltec.display->drawString(0, 12, "Dist: " + String(txData.distance, 1) + " cm");
  Heltec.display->drawString(0, 24, "Level: " + String(txData.waterLevel, 1) + " cm");
  Heltec.display->drawString(0, 36, "Fill: " + String(txData.waterPercent, 1) + "%");
  
  int barWidth = (int)(txData.waterPercent * 1.28);
  Heltec.display->drawRect(0, 48, 128, 12);
  Heltec.display->fillRect(2, 50, barWidth, 8);
  
  Heltec.display->display();
}

// ============================================
// RECEIVER MODE
// ============================================

void setupReceiver() {
  Serial.println("\n=== RECEIVER MODE ===");
  
  rxData.dataValid = false;
  rxData.lastUpdate = 0;
  
  setupWiFi();
  setupLoRa();
  LoRa.receive();
  
  mqttClient.setServer(config.mqttServer, config.mqttPort);
  mqttClient.setCallback(mqttCallback);
  
  displayInfo("Receiver", config.deviceName, "Ready");
  delay(1000);
}

void loopReceiver() {
  // Maintain WiFi
  if (WiFi.status() != WL_CONNECTED) {
    wifiConnected = false;
    displayStatus = "WiFi Lost";
    reconnectWiFi();
  } else {
    wifiConnected = true;
  }
  
  // Maintain MQTT
  if (wifiConnected && !mqttClient.connected()) {
    mqttConnected = false;
    if (millis() - lastMqttAttempt > 5000) {
      reconnectMQTT();
      lastMqttAttempt = millis();
    }
  } else if (wifiConnected) {
    mqttConnected = true;
    mqttClient.loop();
  }
  
  // Check for LoRa packets
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    receiveLoRaData();
  }
  
  // Update display
  if (millis() - lastDisplayUpdate > 500) {
    updateDisplayReceiver();
    lastDisplayUpdate = millis();
  }
}

void receiveLoRaData() {
  String encrypted = "";
  while (LoRa.available()) {
    encrypted += (char)LoRa.read();
  }
  
  rxData.rssi = LoRa.packetRssi();
  String payload = xorDecrypt(encrypted, config.loraKey);
  
  Serial.println("Received: " + payload);
  Serial.println("RSSI: " + String(rxData.rssi));
  
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, payload);
  
  if (error) {
    Serial.println("JSON parse error!");
    displayStatus = "Parse Error";
    return;
  }
  
  rxData.deviceName = doc["device"].as<String>();
  rxData.distance = doc["distance"];
  rxData.level = doc["level"];
  rxData.percent = doc["percent"];
  rxData.battery = doc["battery"];
  rxData.count = doc["count"];
  rxData.lastUpdate = millis();
  rxData.dataValid = true;
  
  displayStatus = "Data Received";
  
  if (mqttConnected) {
    publishToMQTT();
  }
}

void updateDisplayReceiver() {
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_10);
  
  String status = "";
  if (!wifiConnected) {
    status = "WiFi: X";
  } else if (!mqttConnected) {
    status = "MQTT: X";
  } else {
    status = "Connected";
  }
  Heltec.display->drawString(0, 0, "RX: " + status);
  
  if (rxData.dataValid) {
    unsigned long age = (millis() - rxData.lastUpdate) / 1000;
    Heltec.display->drawString(0, 12, rxData.deviceName + " (" + String(age) + "s)");
    Heltec.display->drawString(0, 24, "Level: " + String(rxData.level, 1) + " cm");
    Heltec.display->drawString(0, 36, "Fill: " + String(rxData.percent, 1) + "%");
    Heltec.display->drawString(0, 48, "RSSI:" + String(rxData.rssi) + " " + String(rxData.battery, 1) + "V");
  } else {
    Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
    Heltec.display->drawString(64, 28, "Waiting for data...");
  }
  
  Heltec.display->display();
}

// ============================================
// WIFI & MQTT (Receiver)
// ============================================

void setupWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  
  Serial.print("Connecting to WiFi");
  displayInfo("Connecting", "WiFi...", "");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected: " + WiFi.localIP().toString());
    wifiConnected = true;
  } else {
    Serial.println("\nWiFi failed!");
    wifiConnected = false;
  }
}

void reconnectWiFi() {
  static unsigned long lastAttempt = 0;
  if (millis() - lastAttempt > 10000) {
    Serial.println("Reconnecting WiFi...");
    WiFi.disconnect();
    WiFi.begin();
    lastAttempt = millis();
  }
}

void reconnectMQTT() {
  Serial.print("Connecting to MQTT...");
  displayStatus = "MQTT Connecting";
  
  String clientId = "CisternRX-" + String(config.deviceName);
  bool connected;
  
  if (strlen(config.mqttUser) > 0) {
    connected = mqttClient.connect(clientId.c_str(), config.mqttUser, config.mqttPass);
  } else {
    connected = mqttClient.connect(clientId.c_str());
  }
  
  if (connected) {
    Serial.println("connected!");
    mqttConnected = true;
    sendHADiscovery();
  } else {
    Serial.println("failed, rc=" + String(mqttClient.state()));
    mqttConnected = false;
  }
}

void sendHADiscovery() {
  String deviceId = rxData.deviceName;
  deviceId.replace(" ", "_");
  deviceId.toLowerCase();
  
  String device = "\"device\":{\"identifiers\":[\"" + deviceId + "\"],"
                  "\"name\":\"" + rxData.deviceName + "\","
                  "\"model\":\"LoRa Cistern\",\"manufacturer\":\"DIY\"}";
  
  // Water Level %
  String topic1 = String(config.haDiscoveryPrefix) + "/sensor/" + deviceId + "_percent/config";
  String config1 = "{\"name\":\"" + rxData.deviceName + " Level\","
                   "\"state_topic\":\"" + config.mqttTopic + "/state\","
                   "\"unit_of_measurement\":\"%\","
                   "\"value_template\":\"{{ value_json.percent }}\","
                   "\"unique_id\":\"" + deviceId + "_percent\","
                   "\"icon\":\"mdi:water-percent\"," + device + "}";
  mqttClient.publish(topic1.c_str(), config1.c_str(), true);
  
  // Similar for other sensors...
  Serial.println("HA discovery sent");
}

void publishToMQTT() {
  if (!rxData.dataValid) return;
  
  StaticJsonDocument<300> doc;
  doc["device"] = rxData.deviceName;
  doc["distance"] = rxData.distance;
  doc["level"] = rxData.level;
  doc["percent"] = rxData.percent;
  doc["battery"] = rxData.battery;
  doc["rssi"] = rxData.rssi;
  doc["count"] = rxData.count;
  
  String payload;
  serializeJson(doc, payload);
  
  String stateTopic = String(config.mqttTopic) + "/state";
  if (mqttClient.publish(stateTopic.c_str(), payload.c_str())) {
    Serial.println("Published to MQTT");
    displayStatus = "MQTT Published";
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Handle incoming MQTT if needed
}

// ============================================
// COMMON UTILITIES
// ============================================

void setupLoRa() {
  LoRa.setFrequency(config.loraFreq);
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);
  LoRa.setSyncWord(SYNC_WORD);
  LoRa.enableCrc();
  Serial.println("LoRa initialized");
}

String xorEncrypt(String data, String key) {
  String result = "";
  int keyLen = strlen(config.loraKey) / 2;
  
  for (int i = 0; i < data.length(); i++) {
    int keyByte = hexCharToByte(config.loraKey[2 * (i % keyLen)]) * 16 + 
                  hexCharToByte(config.loraKey[2 * (i % keyLen) + 1]);
    result += (char)(data[i] ^ keyByte);
  }
  return result;
}

String xorDecrypt(String data, String key) {
  return xorEncrypt(data, key); // XOR is symmetric
}

int hexCharToByte(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  return 0;
}

float getBatteryVoltage() {
  // Implement based on your board's battery monitoring
  return 3.7;
}

void displayInfo(String line1, String line2, String line3) {
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawString(64, 10, line1);
  Heltec.display->drawString(64, 25, line2);
  Heltec.display->drawString(64, 40, line3);
  Heltec.display->display();
}
