/**
 * @file LoRa_Tank_Monitor.ino
 * @brief Enhanced LoRa-based tank monitoring system with Home Assistant integration
 *
 * Transmitter: Reads ultrasonic sensor, sends data via LoRa with unique ID
 * Receiver: Receives LoRa data from multiple transmitters, sends to MQTT/Home Assistant
 *
 * BOTH devices need WiFi portal for initial configuration
 */

#define HELTEC_POWER_BUTTON
#include <Arduino.h>
#include "heltec_unofficial.h"
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include <PubSubClient.h>
#include <NewPing.h>
// ADDED: Include for Serial sensor
#include <UltrasonicA02YYUW.h>
#include <map>
#include <vector>
#include <algorithm>

// ============================================================================
// GLOBAL DECLARATIONS
// ============================================================================

const String CURRENT_SKETCH_VERSION = "2.2";  // Updated version for dual sensor support

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;
WebServer webServer(80);
Preferences preferences;
// Network clients
WiFiClient espClient;
PubSubClient client(espClient);

// MQTT settings
String mqtt_server = "192.168.1.103";
int mqtt_port = 1883;
String mqtt_user = "";
String mqtt_pass = "";  // Changed from mqtt_password for consistency

// Home Assistant settings
String ha_discovery_prefix = "homeassistant";
String device_name = "ESP32-LoRa-Device";
String device_identifier = "";
String device_model = "Heltec WiFi LoRa 32 V3";
String device_manufacturer = "Heltec";
String device_version = "2.1";  // Updated version

// ADDED: Sensor type enumeration
enum SensorType {
  SENSOR_NONE,
  SENSOR_PWM,
  SENSOR_SERIAL
};
SensorType sensorType = SENSOR_NONE;

// Ultrasonic sensor settings - Dual type support
#define TRIGGER_PIN  5  // GPIO pin for PWM trigger / Serial TX
#define ECHO_PIN     6  // GPIO pin for PWM echo / Serial RX
#define MAX_DISTANCE 450  // Maximum distance in cm

// ADDED: Dual sensor objects (only one will be used based on detection)
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);  // PWM sensor
UltrasonicA02YYUW serialSensor(Serial1, 5, 6);  // Serial sensor

// LoRa settings - Now configurable via portal
#define PAUSE               30      // Seconds between transmissions
float FREQUENCY = 910.525;
float BANDWIDTH = 250.0;
int SPREADING_FACTOR = 9;
int TRANSMIT_POWER = 1;
int CODING_RATE = 8;

String rxdata;
volatile bool rxFlag = false;
long counter = 0;
uint64_t last_tx = 0;
uint64_t tx_time;
uint64_t minimum_pause;

enum DeviceState {
  STATE_PORTAL,
  STATE_CONNECTED,
  STATE_TRANSMITTING,
  STATE_RECEIVING,
  STATE_ERROR
};
DeviceState currentState = STATE_PORTAL;

bool portalActive = false;
unsigned long portalStartTime = 0;
const unsigned long PORTAL_TIMEOUT = 600000;  // 10 minutes

char displayBuffer[64];

// Device mode - determined by sensor reading
String deviceMode = "Unknown";
String txString = "Ready";
String strPercentFull = "Unknown";
float percentFull = 0.00;
float maxFull = 30.0;
float minFull = 140.0;

// Transmitter variables
String transmitterID = "";
String transmitterName = "";
unsigned long lastTransmitTime = 0;
unsigned long transmitInterval = 30000;  // 30 seconds default

// Receiver variables
struct TransmitterData {
    String distance;
    String percentFull;
    float rssi;
    float snr;
    unsigned long lastSeen;
    String name;
    String id;
    unsigned int packetCount;
    // ADDED: Sensor type for display
    String sensorType;
};

std::map<String, TransmitterData> transmitters;
String currentDisplayTransmitter = "";
unsigned long lastDisplayCycle = 0;
const unsigned long DISPLAY_CYCLE_INTERVAL = 8000; // Cycle every 8 seconds

String apName = "";
unsigned long lastMsg = 0;
unsigned long lastDisplayUpdate = 0;
const unsigned long DISPLAY_UPDATE_INTERVAL = 1000; // Update display every second

// Track if we have valid calibration
bool hasCalibration = false;

// Display management
OLEDDisplay* displayPtr = nullptr;

// Sensor reading optimization
unsigned long lastSensorRead = 0;
const unsigned long SENSOR_READ_INTERVAL = 2000; // Read sensor every 2 seconds
float lastValidDistance = 0.0;
bool sensorReadingValid = false;
std::vector<float> sensorReadings;  // For averaging

// Statistics
unsigned long bootTime = 0;
unsigned long totalPacketsReceived = 0;
unsigned long totalPacketsTransmitted = 0;
unsigned long errorCount = 0;

// Display animation variables
static int scanPos = 0;  // For scanning animation

// Factory reset variables
unsigned long buttonPressStartTime = 0;
bool buttonPressed = false;
const unsigned long FACTORY_RESET_HOLD_TIME = 5000; // 5 seconds

// ============================================================================
// FUNCTION FORWARD DECLARATIONS
// ============================================================================

// Preferences Management
bool isFirstRunAfterUpload();
void resetPreferencesToDefaults();
void forceConfigurationPortal();

// Display Functions
void initDisplay();
void drawTankLevel(float percent, int x, int y, int width, int height);
void drawSignalQuality(float rssi, float snr, int x, int y);
void updateDisplay();
void cycleDisplayTransmitter();
void cleanupOldTransmitters();

// Factory Reset
void checkFactoryReset();

// Configuration Portal
void startConfigurationPortal();

// Device Mode Detection
String determineDeviceMode();

// ADDED: Sensor Functions
SensorType detectSensorType();
float readSensor();
float readSensorEnhanced();
void updateSensorReading();

// MQTT Functions (Receiver only)
void reconnect();
void sendHADiscovery(String txId, String displayName);
void publishSensorData(String txId, String displayName, float distance, float percent, float rssi, float snr);
void setupMQTTDebug();

// LoRa Functions
String prepareLoRaPayload(float distance);
bool parseLoRaPayload(String payload, String& txId, String& txName, float& distance, unsigned long& timestamp, String& sensorType);
void processLoRaPacket();

// ============================================================================
// PREFERENCES MANAGEMENT
// ============================================================================

/**
 * @brief Check if this is first run after upload or version change
 */
bool isFirstRunAfterUpload() {
    preferences.begin("device-config", false);
    String storedVersion = preferences.getString("sketch_version", "");
    bool hasRunBefore = preferences.getBool("has_run_before", false);
    bool hasCalibration = preferences.getBool("has_calibration", false);
    preferences.end();
   
    Serial.printf("Version check: Stored='%s', Current='%s'\n", storedVersion.c_str(), CURRENT_SKETCH_VERSION.c_str());
    Serial.printf("has_run_before: %s, has_calibration: %s\n",
                  hasRunBefore ? "true" : "false",
                  hasCalibration ? "true" : "false");
   
    // Force portal if:
    // 1. No version stored (first run ever)
    // 2. Never run before
    // 3. Version changed (force reconfigure on any version mismatch)
    if (storedVersion == "") {
        Serial.println("First run ever - forcing portal");
        return true;
    }
   
    if (!hasRunBefore) {
        Serial.println("Never run before - forcing portal");
        return true;
    }
   
    if (storedVersion != CURRENT_SKETCH_VERSION) {
        Serial.printf("Version changed (%s -> %s) - forcing portal\n", storedVersion.c_str(), CURRENT_SKETCH_VERSION.c_str());
        return true;
    }
   
    Serial.println("Normal startup - skipping portal");
    return false;
}

/**
 * @brief Reset all preferences to factory defaults
 */
void resetPreferencesToDefaults() {
    Serial.println("Resetting preferences to defaults...");
   
    preferences.begin("device-config", false);
    preferences.clear();
   
    preferences.putString("host", "Tank-Monitor");
    preferences.putFloat("distFull", 30.0);
    preferences.putFloat("distEmpty", 140.0);
    preferences.putString("mqtt_server", "192.168.1.132");
    preferences.putInt("mqtt_port", 1883);
    preferences.putString("mqtt_user", "");
    preferences.putString("mqtt_pass", "");
    preferences.putString("ha_prefix", "homeassistant");
    preferences.putBool("has_run_before", true);
    preferences.putBool("has_calibration", true);
    preferences.putString("sketch_version", CURRENT_SKETCH_VERSION);
    // ADDED: Default sensor type (0 = auto-detect)
    preferences.putInt("sensor_type", 0);
   
    // Default LoRa settings
    preferences.putFloat("lora_freq", 910.525);
    preferences.putInt("lora_sf", 9);
    preferences.putFloat("lora_bw", 250.0);
    preferences.putInt("lora_cr", 8);
    preferences.putInt("lora_power", 1);
   
    preferences.end();
   
    Serial.println("✅ Preferences reset to defaults");
}

/**
 * @brief Force configuration portal (for version upgrades)
 */
void forceConfigurationPortal() {
    Serial.println("\nFORCING CONFIGURATION PORTAL - clearing stored config");
   
    // Clear preferences to ensure a clean configuration state for the portal.
    preferences.begin("device-config", false);
    preferences.clear();
    // Keep an explicit flag to show we forced a portal if needed
    preferences.putString("sketch_version", ""); // make sure isFirstRunAfterUpload sees first-run state if rebooted
    preferences.putBool("has_run_before", false);
    preferences.putBool("has_calibration", false);
    preferences.end();
   
    display.clear();
    display.drawString(0, 0, "FORCE CONFIG");
    display.drawString(0, 15, "Clearing settings...");
    display.drawString(0, 30, "Starting Portal...");
    display.display();
    delay(1000);
   
    startConfigurationPortal();
}

// ============================================================================
// SENSOR DETECTION AND READING FUNCTIONS
// ============================================================================

/**
 * @brief Detect sensor type (PWM or Serial)
 */
/**
 * @brief Detect sensor type (PWM or Serial) - FIXED FOR HELTEC V4
 */
SensorType detectSensorType() {
    Serial.println("\n=== DETECTING SENSOR TYPE ===");
   
    // CRITICAL: Ensure Vext is powered ON for sensor detection
    Serial.println("Powering Vext for sensors...");
    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, LOW);  // LOW powers ON Vext on Heltec
    delay(1500);  // Give sensors time to fully power up
   
    // Test BOTH sensors with proper initialization
    Serial.println("Testing both sensor types...");
   
    // 1. First test PWM sensor
    Serial.println("\n--- Testing PWM Sensor (HC-SR04) ---");
   
    // Initialize PWM pins
    pinMode(TRIGGER_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    digitalWrite(TRIGGER_PIN, LOW);
    delay(100);
   
    int pwmValidReadings = 0;
    float pwmTotal = 0;
   
    for (int i = 0; i < 8; i++) {
        // Manual PWM sensor reading (more reliable than NewPing sometimes)
        digitalWrite(TRIGGER_PIN, LOW);
        delayMicroseconds(2);
        digitalWrite(TRIGGER_PIN, HIGH);
        delayMicroseconds(10);
        digitalWrite(TRIGGER_PIN, LOW);
       
        long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout
        float distance = duration * 0.0343 / 2.0;
       
        Serial.printf("PWM Reading %d: ", i + 1);
        if (duration > 0 && distance >= 2.0 && distance <= MAX_DISTANCE) {
            Serial.printf("%.1f cm (VALID)\n", distance);
            pwmValidReadings++;
            pwmTotal += distance;
        } else {
            if (duration == 0) {
                Serial.println("No echo (timeout)");
            } else {
                Serial.printf("%.1f cm (INVALID - out of range)\n", distance);
            }
        }
        delay(250);  // Longer delay for PWM sensor
    }
   
    // 2. Test Serial sensor
    Serial.println("\n--- Testing Serial Sensor (A02YYUW) ---");
   
    // Initialize Serial1 for sensor
    Serial1.begin(9600, SERIAL_8N1, 5, 6);
    delay(100);
   
    // Clear any existing data
    while (Serial1.available()) {
        Serial1.read();
    }
   
    serialSensor.begin();
    delay(500);
   
    int serialValidReadings = 0;
    float serialTotal = 0;
   
    for (int i = 0; i < 8; i++) {
        serialSensor.update();
        float distance = serialSensor.getDistance();
       
        Serial.printf("Serial Reading %d: ", i + 1);
        if (distance >= 2.0 && distance <= 450.0) {
            Serial.printf("%.1f cm (VALID)\n", distance);
            serialValidReadings++;
            serialTotal += distance;
        } else {
            Serial.printf("%.1f cm (INVALID)\n", distance);
        }
        delay(250);  // Longer delay for serial sensor
    }
   
    // Calculate averages
    float pwmAvg = (pwmValidReadings > 0) ? pwmTotal / pwmValidReadings : 0;
    float serialAvg = (serialValidReadings > 0) ? serialTotal / serialValidReadings : 0;
   
    Serial.println("\n=== DETECTION RESULTS ===");
    Serial.printf("PWM: %d/%d valid readings, Avg: %.1f cm\n",
                  pwmValidReadings, 8, pwmAvg);
    Serial.printf("Serial: %d/%d valid readings, Avg: %.1f cm\n",
                  serialValidReadings, 8, serialAvg);
   
    // Decision logic with thresholds
    const int MIN_VALID_READINGS = 4;  // Need at least 4 valid readings
   
    bool hasPWM = (pwmValidReadings >= MIN_VALID_READINGS);
    bool hasSerial = (serialValidReadings >= MIN_VALID_READINGS);
   
    if (hasPWM && !hasSerial) {
        Serial.println("✅ PWM SENSOR DETECTED (HC-SR04 type)");
        Serial.println("   Make sure: VCC→Vext, GND→GND, TRIG→GPIO5, ECHO→GPIO6");
        return SENSOR_PWM;
    }
    else if (hasSerial && !hasPWM) {
        Serial.println("✅ SERIAL SENSOR DETECTED (A02YYUW type)");
        Serial.println("   Make sure: VCC→Vext, GND→GND, TX→GPIO5, RX→GPIO6");
        return SENSOR_SERIAL;
    }
    else if (hasPWM && hasSerial) {
        // Both sensors returning readings - check preferences
        int prefType = preferences.getInt("sensor_type", 0);
        if (prefType == 2) {  // 2 = Serial
            Serial.println("⚠️  Both sensors detected, using SERIAL (from preferences)");
            return SENSOR_SERIAL;
        } else if (prefType == 1) {  // 1 = PWM
            Serial.println("⚠️  Both sensors detected, using PWM (from preferences)");
            return SENSOR_PWM;
        } else {
            // Auto-decide: whichever has more valid readings
            if (pwmValidReadings > serialValidReadings) {
                Serial.println("⚠️  Both sensors detected, using PWM (more valid readings)");
                return SENSOR_PWM;
            } else {
                Serial.println("⚠️  Both sensors detected, using SERIAL (more valid readings)");
                return SENSOR_SERIAL;
            }
        }
    }
    else if (pwmValidReadings > 0 && serialValidReadings == 0) {
        // Some PWM readings but not enough for definite detection
        Serial.println("⚠️  WEAK PWM SIGNAL DETECTED");
        Serial.println("   Possible issues:");
        Serial.println("   1. Sensor not getting enough power");
        Serial.println("   2. Wiring incorrect (TRIG/ECHO swapped?)");
        Serial.println("   3. Object too close/far (needs 2-400cm object)");
        Serial.println("   Defaulting to PWM sensor...");
        return SENSOR_PWM;
    }
    else {
        Serial.println("❌ NO VALID SENSOR DETECTED");
        Serial.println("   Check:");
        Serial.println("   1. Sensor connected to Vext pin (not 5V!)");
        Serial.println("   2. GND connected");
        Serial.println("   3. TRIG/TX to GPIO5");
        Serial.println("   4. ECHO/RX to GPIO6");
        Serial.println("   5. Object 2-400cm from sensor");
        return SENSOR_NONE;
    }
}
/**
 * @brief Read sensor based on detected type
 */
float readSensor() {
    if (sensorType == SENSOR_PWM) {
        // Read PWM sensor
        unsigned int distance = sonar.ping_cm();
        if (distance >= 2 && distance <= MAX_DISTANCE) {
            return (float)distance;
        }
    } else if (sensorType == SENSOR_SERIAL) {
        // Read Serial sensor
        serialSensor.update();
        float distance = serialSensor.getDistance();
        if (distance >= 2.0 && distance <= 450.0) {
            return distance;
        }
    }
   
    return -1.0;  // Invalid reading
}

/**
 * @brief Read sensor with multiple samples and averaging
 */
float readSensorEnhanced() {
    // Power on sensor for reading
    digitalWrite(Vext, LOW);
    delay(50);
   
    const int numSamples = 10;
    std::vector<float> samples;
   
    for (int i = 0; i < numSamples; i++) {
        float distance = readSensor();
       
        // Validate reading
        if (distance >= 2.0 && distance <= 450.0) {
            samples.push_back(distance);
        }
       
        if (i < numSamples - 1) {
            delay(100);
        }
    }
   
    // Need at least 5 valid readings
    if (samples.size() < 5) {
        Serial.println("Sensor error: insufficient valid readings");
        return -1.0;
    }
   
    // Sort and get median
    std::sort(samples.begin(), samples.end());
    float median = samples[samples.size() / 2];
   
    // Filter outliers (more than 20% from median)
    std::vector<float> filtered;
    for (float sample : samples) {
        if (fabs(sample - median) / median < 0.2) {
            filtered.push_back(sample);
        }
    }
   
    // Calculate average of filtered readings
    float sum = 0;
    for (float sample : filtered) {
        sum += sample;
    }
   
    float average = sum / filtered.size();
    Serial.printf("Sensor (%s): %.1f cm (filtered from %d samples)\n",
                  sensorType == SENSOR_PWM ? "PWM" : "Serial",
                  average, filtered.size());
   
    return average;
}

/**
 * @brief Update sensor reading with enhanced processing
 */
void updateSensorReading() {
    if (deviceMode != "Transmitter") return;
   
    static unsigned long lastSensorUpdate = 0;
    unsigned long now = millis();
   
    // Only read sensor at fixed interval
    if (now - lastSensorUpdate < SENSOR_READ_INTERVAL) {
        return;
    }
    lastSensorUpdate = now;
   
    float distance = readSensorEnhanced();
   
    if (distance > 0) {
        // Store reading for averaging
        sensorReadings.push_back(distance);
        if (sensorReadings.size() > 10) {
            sensorReadings.erase(sensorReadings.begin());
        }
       
        // Calculate moving average
        float sum = 0;
        for (float reading : sensorReadings) {
            sum += reading;
        }
        float average = sum / sensorReadings.size();
       
        lastValidDistance = average;
        sensorReadingValid = true;
       
        // Calculate percentage
        if (hasCalibration) {
            percentFull = 100.0 * (minFull - average) / (minFull - maxFull);
            percentFull = constrain(percentFull, 0.0, 100.0);
            strPercentFull = String(percentFull, 1);
        }
       
    } else {
        sensorReadingValid = false;
        errorCount++;
        Serial.println("Sensor reading failed");
    }
   
    lastSensorRead = now;
}

// ============================================================================
// DEVICE MODE DETECTION
// ============================================================================

/**
 * @brief Determine if device is transmitter or receiver
 * @return "Transmitter" or "Receiver"
 */
String determineDeviceMode() {
    Serial.println("\n=== DETERMINING DEVICE MODE ===");
   
    // First detect sensor type
    sensorType = detectSensorType();
   
    if (sensorType == SENSOR_NONE) {
        Serial.println("❌ NO SENSOR DETECTED - Assuming RECEIVER");
        return "Receiver";
    }
   
    Serial.printf("Using %s sensor for mode detection\n",
                  sensorType == SENSOR_PWM ? "PWM" : "Serial");
   
    // Take multiple readings to be sure
    const int numReadings = 10;
    int validReadings = 0;
    std::vector<float> readings;
   
    Serial.println("\nTaking sensor readings...");
   
    for (int i = 0; i < numReadings; i++) {
        float distance = readSensor();
       
        Serial.printf("Reading %d: %.1f cm - ", i + 1, distance);
        readings.push_back(distance);
       
        // Check if reading is valid
        if (distance >= 2.0 && distance <= 450.0) {
            validReadings++;
            Serial.println("VALID");
        } else if (distance == 0.0) {
            Serial.println("INVALID (0 cm - no echo/reading)");
        } else if (distance < 0) {
            Serial.println("INVALID (negative value)");
        } else {
            Serial.println("INVALID (out of range)");
        }
       
        delay(200);
    }
   
    // Analyze the readings
    Serial.println("\n=== SENSOR ANALYSIS ===");
    Serial.printf("Valid readings: %d/%d\n", validReadings, numReadings);
   
    // Calculate statistics
    float minVal = 10000.0, maxVal = -10000.0, sum = 0.0;
    for (float val : readings) {
        if (val < minVal) minVal = val;
        if (val > maxVal) maxVal = val;
        sum += val;
    }
    float avg = sum / readings.size();
   
    Serial.printf("Range: %.1f to %.1f cm\n", minVal, maxVal);
    Serial.printf("Average: %.1f cm\n", avg);
   
    // Decision logic
    bool isTransmitter = false;
   
    if (validReadings >= 5) {
        isTransmitter = true;
        Serial.println("✅ SUFFICIENT VALID READINGS");
    } else if (maxVal - minVal > 10.0 && avg > 10.0) {
        isTransmitter = true;
        Serial.println("✅ SIGNIFICANT READING VARIATION DETECTED");
    } else {
        Serial.println("❌ INSUFFICIENT/INVALID READINGS");
    }
   
    if (isTransmitter) {
        Serial.printf("\n✅ TRANSMITTER MODE DETECTED\n");
        Serial.printf("   Sensor type: %s\n", sensorType == SENSOR_PWM ? "PWM" : "Serial");
        Serial.printf("   Valid readings: %d/%d\n", validReadings, numReadings);
        Serial.printf("   Reading range: %.1f - %.1f cm\n", minVal, maxVal);
        return "Transmitter";
    } else {
        Serial.printf("\n⚠️  RECEIVER MODE DETECTED\n");
        Serial.println("   Possible issues:");
        Serial.println("   1. Sensor not properly connected");
        Serial.println("   2. Sensor not powered (check Vext connection)");
        Serial.println("   3. Sensor faulty");
        Serial.println("   4. No object within range (needs 2-450cm object)");
        return "Receiver";
    }
}

// ============================================================================
// DISPLAY INITIALIZATION (Always On)
// ============================================================================

/**
 * @brief Initialize display (always powered on)
 */
void initDisplay() {
    // Power on the display (Vext controls both display and sensor on Heltec V3)
    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, LOW);
    delay(100);  // Give display time to power up
   
    // Initialize display
    display.init();
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
   
    // Show startup message
    display.drawString(0, 0, "Tank Monitor v" + CURRENT_SKETCH_VERSION);
    display.drawString(0, 15, "Mode: Detecting...");
    display.display();
   
    Serial.println("Display initialized (always on)");
}

/**
 * @brief Draw a tank level graphic optimized for 64px height with better text visibility
 */
void drawTankLevel(float percent, int x, int y, int width, int height) {
    // Tank outline (smaller for limited space)
    display.drawRect(x, y, width, height);
   
    // Fill level
    int fillHeight = (percent / 100.0) * height;
    int fillY = y + height - fillHeight;
   
    // Different fill styles based on level
    if (percent > 75) {
        // Green - high level (striped for better text visibility)
        for (int i = 0; i < fillHeight; i += 3) {
            display.drawHorizontalLine(x + 1, fillY + i, width - 2);
        }
    } else if (percent > 25) {
        // Yellow stripes - medium level
        for (int i = 0; i < fillHeight; i += 2) {
            display.drawHorizontalLine(x + 1, fillY + i, width - 2);
        }
    } else {
        // Red stripes - low level
        for (int i = 0; i < fillHeight; i += 3) {
            display.drawHorizontalLine(x + 1, fillY + i, width - 2);
        }
    }
   
    // Small percentage text inside tank - HIGH CONTRAST VERSION
    // Create a white background box for the text
    int textWidth = 20; // Width enough for "100%"
    int textHeight = 12;
    int textX = x + (width - textWidth) / 2;
    int textY = y + (height - textHeight) / 2;
   
    // Draw white background for text (inverted on OLED)
    display.fillRect(textX, textY, textWidth, textHeight);
   
    // Draw black text on white background
    display.setColor(BLACK);
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    snprintf(displayBuffer, sizeof(displayBuffer), "%.0f%%", percent);
    display.drawString(x + width / 2, y + height/2 - 5, displayBuffer);
   
    // Reset to white for other drawing
    display.setColor(WHITE);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
}

/**
 * @brief Draw compact signal quality indicator (RSSI + SNR)
 */
void drawSignalQuality(float rssi, float snr, int x, int y) {
    // Convert RSSI to bars (0-3 for compact display)
    int bars = 0;
    if (rssi > -70) bars = 3;
    else if (rssi > -80) bars = 2;
    else if (rssi > -90) bars = 1;
   
    // Draw compact bars (3 bars max)
    for (int i = 0; i < 3; i++) {
        int barHeight = (i + 1) * 3;
        int barY = y + (9 - barHeight);
       
        if (i < bars) {
            // Filled bar
            display.fillRect(x + i * 6, barY, 4, barHeight);
        } else {
            // Empty bar
            display.drawRect(x + i * 6, barY, 4, barHeight);
        }
    }
   
    // Add SNR quality dot next to bars
    // Good SNR: >= 7.0 (green dot)
    // Fair SNR: >= 0.0 (yellow dot)  
    // Poor SNR: < 0.0 (red dot)
    int dotX = x + 20; // Space after bars
    int dotY = y + 4;
   
    if (snr >= 7.0) {
        // Good SNR - filled green circle
        display.fillCircle(dotX, dotY, 2);
    } else if (snr >= 0.0) {
        // Fair SNR - circle outline
        display.drawCircle(dotX, dotY, 2);
    } else {
        // Poor SNR - X mark
        display.drawLine(dotX-2, dotY-2, dotX+2, dotY+2);
        display.drawLine(dotX+2, dotY-2, dotX-2, dotY+2);
    }
}

/**
 * @brief Cycle through transmitters on display (receiver only)
 */
void cycleDisplayTransmitter() {
    if (deviceMode != "Receiver") return;
   
    unsigned long now = millis();
   
    if (now - lastDisplayCycle < DISPLAY_CYCLE_INTERVAL || transmitters.empty()) {
        return;
    }
    lastDisplayCycle = now;
   
    // Find next transmitter in the map
    auto it = transmitters.find(currentDisplayTransmitter);
    if (it != transmitters.end()) {
        ++it;
        if (it == transmitters.end()) {
            it = transmitters.begin();
        }
        currentDisplayTransmitter = it->first;
    } else {
        // Current transmitter not found, start from beginning
        if (!transmitters.empty()) {
            currentDisplayTransmitter = transmitters.begin()->first;
        }
    }
   
    Serial.printf("Now displaying transmitter: %s\n", currentDisplayTransmitter.c_str());
}

/**
 * @brief Clean up old transmitters (receiver only)
 */
void cleanupOldTransmitters() {
    if (deviceMode != "Receiver") return;
   
    unsigned long now = millis();
    for (auto it = transmitters.begin(); it != transmitters.end(); ) {
        if (now - it->second.lastSeen > 300000) {  // 5 minutes
            Serial.printf("Removing old transmitter: %s\n", it->first.c_str());
            it = transmitters.erase(it);
        } else {
            ++it;
        }
    }
}

/**
 * @brief SINGLE CONSOLIDATED DISPLAY UPDATE FUNCTION
 * Optimized for 128x64 OLED display with better spacing
 */
void updateDisplay() {
    static unsigned long lastDisplayUpdate = 0;
    unsigned long now = millis();
   
    // Only update display at fixed interval (1000ms)
    if (now - lastDisplayUpdate < DISPLAY_UPDATE_INTERVAL) {
        return;
    }
    lastDisplayUpdate = now;
   
    // Clear display buffer
    display.clear();
   
    if (deviceMode == "Receiver") {
        // ============================================
        // RECEIVER MODE - Compact display for 128x64
        // ============================================
       
        // Header line (Line 0-10px)
        display.setFont(ArialMT_Plain_10);
        display.drawString(0, 0, "RX");
       
        // WiFi status indicator
        display.drawString(20, 0, WiFi.status() == WL_CONNECTED ? "WiFi✓" : "WiFi✗");
       
        // Transmitter count
        snprintf(displayBuffer, sizeof(displayBuffer), "TX:%d", transmitters.size());
        display.drawString(60, 0, displayBuffer);
       
        // Uptime in minutes
        snprintf(displayBuffer, sizeof(displayBuffer), "Up:%lum", (now - bootTime) / 60000);
        display.drawString(90, 0, displayBuffer);
       
        // Divider line
        display.drawHorizontalLine(0, 12, 128);
       
        if (!transmitters.empty() && currentDisplayTransmitter != "") {
            auto it = transmitters.find(currentDisplayTransmitter);
            if (it != transmitters.end()) {
                const TransmitterData& data = it->second;
               
                // Transmitter name (Line 13-23px)
                String displayName = data.name;
                if (displayName.length() > 12) {
                    displayName = displayName.substring(0, 12) + "...";
                }
                display.drawString(0, 13, displayName);
               
                // ADDED: Sensor type indicator
                if (data.sensorType == "PWM" || data.sensorType == "Serial") {
                    display.setFont(ArialMT_Plain_10);
                    display.drawString(100, 13, data.sensorType.substring(0, 3));
                }
               
                // Tank level graphic (Lines 25-45px, moved to right side)
                float percent = data.percentFull.toFloat();
                drawTankLevel(percent, 95, 15, 25, 30);  // Moved further right
               
                // Signal quality with SNR indicator (Line 25-35px)
                display.drawString(0, 25, "Sig:");
                drawSignalQuality(data.rssi, data.snr, 25, 25);
               
                // Distance (Line 38-48px)
                snprintf(displayBuffer, sizeof(displayBuffer), "Dist:%scm", data.distance.c_str());
                display.drawString(0, 38, displayBuffer);
               
                // SHOW HOST/IP INSTEAD OF ID (Line 50-60px, left side)
                String hostInfo = "";
                if (WiFi.status() == WL_CONNECTED) {
                    IPAddress ip = WiFi.localIP();
                    // Show last octet of IP (e.g., "132" for 192.168.1.132)
                    hostInfo = String(ip[3]);
                } else {
                    hostInfo = "No WiFi";
                }
                snprintf(displayBuffer, sizeof(displayBuffer), "Host:%s", hostInfo.c_str());
                display.drawString(0, 50, displayBuffer);
               
                // Time ago (Line 50-60px, right side)
                unsigned long age = (now - data.lastSeen) / 1000;
                if (age < 60) {
                    snprintf(displayBuffer, sizeof(displayBuffer), "%lus ago", age);
                } else if (age < 3600) {
                    snprintf(displayBuffer, sizeof(displayBuffer), "%lum ago", age / 60);
                } else {
                    snprintf(displayBuffer, sizeof(displayBuffer), "%luh ago", age / 3600);
                }
                display.drawString(70, 50, displayBuffer);
               
            }
        } else {
            // No transmitters found - show scanning animation
            display.drawString(0, 25, "Scanning...");
           
            // Scanning animation
            display.drawHorizontalLine(0, 40, 128);
            display.fillRect(scanPos, 38, 10, 4);
            scanPos = (scanPos + 5) % 128;
           
            // Show host/IP info even when no transmitters
            String hostInfo = "";
            if (WiFi.status() == WL_CONNECTED) {
                IPAddress ip = WiFi.localIP();
                hostInfo = String(ip[3]);  // Show last octet
            } else {
                hostInfo = "No WiFi";
            }
            snprintf(displayBuffer, sizeof(displayBuffer), "Host:%s", hostInfo.c_str());
            display.drawString(0, 50, displayBuffer);
           
            // Show packet statistics
            snprintf(displayBuffer, sizeof(displayBuffer), "Pkts:%lu", totalPacketsReceived);
            display.drawString(70, 50, displayBuffer);
        }
       
    } else {
        // ============================================
        // TRANSMITTER MODE - Compact display for 128x64
        // ============================================
       
        // Header line (Line 0-10px)
        display.setFont(ArialMT_Plain_10);
        display.drawString(0, 0, "TX");
       
        // Transmitter ID (shortened)
        String shortID = transmitterID;
        if (shortID.length() > 6) {
            shortID = shortID.substring(0, 6);
        }
        snprintf(displayBuffer, sizeof(displayBuffer), "ID:%s", shortID.c_str());
        display.drawString(25, 0, displayBuffer);
       
        // ADDED: Sensor type indicator
        String sensorTypeStr = sensorType == SENSOR_PWM ? "PWM" :
                              sensorType == SENSOR_SERIAL ? "SER" : "NONE";
        display.drawString(70, 0, sensorTypeStr);
       
        // Divider line
        display.drawHorizontalLine(0, 12, 128);
       
        if (sensorReadingValid) {
            // Tank level graphic (Lines 15-45px, centered)
            float percent = percentFull;
            drawTankLevel(percent, 95, 15, 25, 30);
           
            // Distance reading (Line 15-25px)
            snprintf(displayBuffer, sizeof(displayBuffer), "Dist:%.1fcm", lastValidDistance);
            display.drawString(0, 15, displayBuffer);
           
            // Percentage (Line 28-38px)
            snprintf(displayBuffer, sizeof(displayBuffer), "Fill:%.1f%%", percent);
            display.drawString(0, 28, displayBuffer);
           
            // Next transmission time (Line 41-51px)
            unsigned long nextTx = max(0L, (long)(transmitInterval - (millis() - lastTransmitTime)) / 1000);
            snprintf(displayBuffer, sizeof(displayBuffer), "Next:%lus", nextTx);
            display.drawString(0, 41, displayBuffer);
           
            // Packet count (Line 54-64px)
            snprintf(displayBuffer, sizeof(displayBuffer), "Pkts:%lu", totalPacketsTransmitted);
            display.drawString(0, 54, displayBuffer);
           
        } else {
            // Sensor error state
            display.drawString(0, 20, "⚠️ Sensor Error");
            display.drawString(0, 35, "Check connection");
           
            // Show last valid reading if available
            if (lastValidDistance > 0) {
                snprintf(displayBuffer, sizeof(displayBuffer), "Last:%.1fcm", lastValidDistance);
                display.drawString(0, 50, displayBuffer);
            }
           
            // Show transmitter ID in error state
            String shortID = transmitterID;
            if (shortID.length() > 6) {
                shortID = shortID.substring(0, 6);
            }
            snprintf(displayBuffer, sizeof(displayBuffer), "ID:%s", shortID.c_str());
            display.drawString(70, 50, displayBuffer);
        }
    }
   
    // Update display once (minimal flicker)
    display.display();
}

// ============================================================================
// FACTORY RESET FUNCTION
// ============================================================================

/**
 * @brief Check for factory reset button press
 */
void checkFactoryReset() {
    // For Heltec WiFi LoRa 32 V3, the button is on GPIO 0
    const int BUTTON_PIN = 0;
   
    // Check if button is being held for factory reset
    if (digitalRead(BUTTON_PIN) == LOW) { // Button pressed
        if (!buttonPressed) {
            buttonPressed = true;
            buttonPressStartTime = millis();
            Serial.println("Button pressed - hold for 5s to factory reset");
           
            // Show countdown on display
            display.clear();
            display.drawString(0, 0, "HOLD BUTTON");
            display.drawString(0, 15, "5s: Factory Reset");
            display.drawString(0, 30, "Release to cancel");
            display.display();
        } else {
            // Button still being held
            unsigned long holdTime = millis() - buttonPressStartTime;
            int secondsLeft = 5 - (holdTime / 1000);
           
            if (secondsLeft >= 0) {
                // Update countdown
                display.clear();
                display.drawString(0, 0, "HOLDING FOR RESET");
                display.drawString(0, 15, String(secondsLeft) + " seconds...");
                display.drawString(0, 30, "Release to cancel");
                display.display();
            }
           
            if (holdTime >= FACTORY_RESET_HOLD_TIME) {
                Serial.println("Factory reset triggered!");
               
                display.clear();
                display.drawString(0, 0, "FACTORY RESET");
                display.drawString(0, 15, "Clearing settings...");
                display.display();
               
                resetPreferencesToDefaults();
                delay(1000);
               
                display.drawString(0, 30, "Restarting...");
                display.display();
                delay(2000);
               
                ESP.restart();
            }
        }
    } else {
        if (buttonPressed) {
            buttonPressed = false;
            Serial.println("Button released - factory reset cancelled");
           
            // Restore normal display
            updateDisplay();
        }
    }
}

// ============================================================================
// ENHANCED CONFIGURATION PORTAL WITH LoRa SETTINGS AND SENSOR TYPE
// ============================================================================

void startConfigurationPortal() {
    Serial.println("\nStarting configuration portal...");
   
    preferences.begin("device-config", false);
    String host = preferences.getString("host", "Tank-Monitor");
    preferences.end();
   
    apName = host + "-Config";
   
    // DISABLE WiFi for transmitter to prevent interference
    if (deviceMode == "Transmitter") {
        Serial.println("Transmitter mode: WiFi will be DISABLED after configuration");
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        delay(100);
    }
   
    WiFi.mode(WIFI_AP);
    delay(100);
   
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
   
    bool apStarted = WiFi.softAP(apName.c_str());
   
    if (apStarted) {
        Serial.println("Access Point started successfully!");
        Serial.println("  SSID: " + apName);
        Serial.println("  IP: 192.168.4.1");
        Serial.println("  No password");
       
        dnsServer.start(DNS_PORT, "*", apIP);
       
        // Setup web server routes
        webServer.on("/", HTTP_GET, [host]() {
            String html = "<!DOCTYPE html><html><head>";
            html += "<meta charset='UTF-8'>";
            html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
            html += "<meta http-equiv='X-UA-Compatible' content='ie=edge'>";
            html += "<title>Tank Monitor Configuration</title>";
            html += "<style>";
            html += "* { box-sizing: border-box; }";
            html += "body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, 'Helvetica Neue', Arial, sans-serif; ";
            html += "margin: 0; padding: 0; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); min-height: 100vh; }";
            html += ".container { max-width: 600px; margin: 20px auto; background: white; border-radius: 12px; ";
            html += "box-shadow: 0 10px 40px rgba(0,0,0,0.2); overflow: hidden; }";
            html += ".header { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; ";
            html += "padding: 30px 20px; text-align: center; }";
            html += ".header h1 { margin: 0 0 5px 0; font-size: 28px; font-weight: 600; }";
            html += ".header p { margin: 5px 0 0 0; font-size: 14px; opacity: 0.9; }";
            html += ".content { padding: 30px 20px; }";
            html += ".info-box { background: #e3f2fd; border-left: 4px solid #2196F3; padding: 15px; ";
            html += "border-radius: 4px; margin-bottom: 20px; font-size: 14px; }";
            html += ".info-box strong { display: block; color: #1976D2; margin-bottom: 5px; }";
            html += ".tabs { display: flex; flex-wrap: wrap; gap: 0; margin-bottom: 20px; border-bottom: 2px solid #e0e0e0; }";
            html += ".tab { padding: 12px 16px; background: none; border: none; cursor: pointer; font-size: 14px; ";
            html += "font-weight: 500; color: #666; transition: all 0.3s; border-bottom: 3px solid transparent; }";
            html += ".tab:hover { color: #667eea; }";
            html += ".tab.active { color: #667eea; border-bottom-color: #667eea; }";
            html += ".tab-content { display: none; }";
            html += ".tab-content.active { display: block; animation: fadeIn 0.3s; }";
            html += "@keyframes fadeIn { from { opacity: 0; } to { opacity: 1; } }";
            html += "label { display: block; margin: 20px 0 8px 0; font-weight: 600; color: #333; font-size: 14px; }";
            html += ".required::after { content: '*'; color: #f44336; margin-left: 4px; }";
            html += "input[type='text'], input[type='password'], input[type='number'], select { ";
            html += "width: 100%; padding: 12px 14px; margin-bottom: 15px; border: 2px solid #e0e0e0; ";
            html += "border-radius: 6px; font-size: 14px; font-family: inherit; transition: border-color 0.3s; }";
            html += "input[type='text']:focus, input[type='password']:focus, input[type='number']:focus, select:focus { ";
            html += "outline: none; border-color: #667eea; background-color: #f5f7ff; }";
            html += ".input-group { margin-bottom: 15px; }";
            html += ".input-hint { font-size: 12px; color: #999; margin-top: -10px; margin-bottom: 10px; }";
            html += ".password-wrapper { display: flex; gap: 6px; margin-bottom: 15px; }";
            html += ".password-wrapper input { flex: 1; margin-bottom: 0; }";
            html += ".password-toggle { padding: 12px 14px; background: #f5f5f5; border: 2px solid #e0e0e0; ";
            html += "border-radius: 6px; cursor: pointer; font-size: 12px; font-weight: 500; color: #666; ";
            html += "white-space: nowrap; transition: all 0.3s; }";
            html += ".password-toggle:hover { background: #efefef; border-color: #d0d0d0; }";
            html += ".password-toggle:active { background: #e0e0e0; }";
            html += ".preset-group { display: flex; flex-wrap: wrap; gap: 8px; margin: 15px 0 20px 0; }";
            html += ".preset-btn { padding: 10px 14px; background: #f5f5f5; border: 2px solid #e0e0e0; ";
            html += "border-radius: 6px; cursor: pointer; font-size: 13px; font-weight: 500; color: #333; ";
            html += "transition: all 0.3s; white-space: nowrap; }";
            html += ".preset-btn:hover { background: #efefef; border-color: #d0d0d0; }";
            html += ".preset-btn:active { background: #e0e0e0; }";
            html += ".preset-btn-rf { padding: 10px 14px; background: #2196F3; border: 2px solid #1976D2; ";
            html += "border-radius: 6px; cursor: pointer; font-size: 13px; font-weight: 500; color: white; ";
            html += "transition: all 0.3s; white-space: nowrap; }";
            html += ".preset-btn-rf:hover { background: #1976D2; border-color: #1565C0; }";
            html += ".preset-btn-rf:active { background: #1565C0; }";
            html += ".section-divider { height: 1px; background: #e0e0e0; margin: 25px 0; }";
            html += ".button-group { display: flex; gap: 10px; }";
            html += "button[type='submit'] { flex: 1; padding: 14px 20px; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); ";
            html += "color: white; border: none; border-radius: 6px; cursor: pointer; font-size: 16px; font-weight: 600; ";
            html += "transition: transform 0.2s, box-shadow 0.2s; }";
            html += "button[type='submit']:hover { transform: translateY(-2px); box-shadow: 0 5px 20px rgba(102, 126, 234, 0.4); }";
            html += "button[type='submit']:active { transform: translateY(0); }";
            html += ".error-message { background: #ffebee; border-left: 4px solid #f44336; color: #c62828; ";
            html += "padding: 12px; border-radius: 4px; margin-bottom: 15px; font-size: 13px; }";
            html += ".success-message { background: #e8f5e9; border-left: 4px solid #4caf50; color: #2e7d32; ";
            html += "padding: 12px; border-radius: 4px; margin-bottom: 15px; font-size: 13px; }";
            html += ".wiring-box { background: #fff3e0; border-left: 4px solid #ff9800; padding: 15px; ";
            html += "border-radius: 4px; margin: 15px 0; font-size: 13px; }";
            html += ".wiring-box p { margin: 8px 0; font-family: 'Courier New', monospace; }";
            html += ".wiring-box strong { display: block; margin: 10px 0 5px 0; }";
            html += "@media (max-width: 600px) {";
            html += ".tabs { gap: 0; }";
            html += ".tab { flex: 1; padding: 10px 8px; font-size: 12px; }";
            html += ".preset-group { gap: 5px; }";
            html += ".preset-btn, .preset-btn-rf { padding: 8px 10px; font-size: 12px; }";
            html += "}";
            html += "</style>";
            html += "<script>";
            html += "function showTab(tabName) {";
            html += "  const tabs = document.querySelectorAll('.tab-content');";
            html += "  tabs.forEach(t => t.classList.remove('active'));";
            html += "  const buttons = document.querySelectorAll('.tab');";
            html += "  buttons.forEach(b => b.classList.remove('active'));";
            html += "  const content = document.getElementById(tabName);";
            html += "  if (content) { content.classList.add('active'); }";
            html += "  event.target.classList.add('active');";
            html += "}";
            html += "function setTankPreset(full, empty) {";
            html += "  document.querySelector('input[name=\"distFull\"]').value = full;";
            html += "  document.querySelector('input[name=\"distEmpty\"]').value = empty;";
            html += "  showValidation();";
            html += "}";
            html += "function setLoRaPreset(preset) {";
            html += "  const presets = {";
            html += "    'balanced': { freq: '910.525', sf: '9', bw: '250', power: '1', cr: '8' },";
            html += "    'long_range': { freq: '910.525', sf: '12', bw: '125', power: '2', cr: '8' },";
            html += "    'fast': { freq: '910.525', sf: '7', bw: '500', power: '1', cr: '5' },";
            html += "    'low_power': { freq: '910.525', sf: '9', bw: '125', power: '0', cr: '8' }";
            html += "  };";
            html += "  if (presets[preset]) {";
            html += "    const p = presets[preset];";
            html += "    document.querySelector('input[name=\"lora_freq\"]').value = p.freq;";
            html += "    document.querySelector('select[name=\"lora_sf\"]').value = p.sf;";
            html += "    document.querySelector('select[name=\"lora_bw\"]').value = p.bw;";
            html += "    document.querySelector('select[name=\"lora_power\"]').value = p.power;";
            html += "    document.querySelector('select[name=\"lora_cr\"]').value = p.cr;";
            html += "  }";
            html += "}";
            html += "function togglePasswordVisibility(inputId, button) {";
            html += "  const input = document.getElementById(inputId);";
            html += "  if (input.type === 'password') {";
            html += "    input.type = 'text';";
            html += "    button.textContent = 'Hide';";
            html += "  } else {";
            html += "    input.type = 'password';";
            html += "    button.textContent = 'Show';";
            html += "  }";
            html += "}";
            html += "function validateForm() {";
            html += "  const full = parseFloat(document.querySelector('input[name=\"distFull\"]').value);";
            html += "  const empty = parseFloat(document.querySelector('input[name=\"distEmpty\"]').value);";
            html += "  const freq = parseFloat(document.querySelector('input[name=\"lora_freq\"]').value);";
            html += "  if (isNaN(full) || isNaN(empty)) {";
            html += "    alert('ERROR: Tank distances must be numbers');";
            html += "    return false;";
            html += "  }";
            html += "  if (full >= empty) {";
            html += "    alert('ERROR: Full distance (\" + full + \"cm) must be LESS than Empty distance (\" + empty + \"cm)\\n\\nExample: Full=30cm, Empty=140cm');";
            html += "    return false;";
            html += "  }";
            html += "  if (full < 2 || empty > 450) {";
            html += "    alert('ERROR: Distance values out of sensor range (2-450cm)');";
            html += "    return false;";
            html += "  }";
            html += "  if (isNaN(freq) || freq < 902 || freq > 928) {";
            html += "    alert('ERROR: Frequency must be between 902-928 MHz');";
            html += "    return false;";
            html += "  }";
            html += "  return true;";
            html += "}";
            html += "function showValidation() {";
            html += "  const full = parseFloat(document.querySelector('input[name=\"distFull\"]').value);";
            html += "  const empty = parseFloat(document.querySelector('input[name=\"distEmpty\"]').value);";
            html += "  const status = document.getElementById('calibration-status');";
            html += "  if (status && !isNaN(full) && !isNaN(empty)) {";
            html += "    if (full < empty) {";
            html += "      status.innerHTML = '<div class=\"success-message\">Calibration values valid!</div>';";
            html += "    } else {";
            html += "      status.innerHTML = '<div class=\"error-message\">Full must be less than Empty</div>';";
            html += "    }";
            html += "  }";
            html += "}";
            html += "document.addEventListener('DOMContentLoaded', function() {";
            html += "  document.querySelector('.tab').click();";
            html += "  showValidation();";
            html += "  document.querySelector('input[name=\"distFull\"]').addEventListener('change', showValidation);";
            html += "  document.querySelector('input[name=\"distEmpty\"]').addEventListener('change', showValidation);";
            html += "});";
            html += "</script>";
            html += "</head><body>";
            html += "<div class='container'>";
           
            // Header
            html += "<div class='header'>";
            html += "<h1>Tank Monitor Setup</h1>";
            html += "<p>v" + String(CURRENT_SKETCH_VERSION) + "</p>";
            html += "</div>";
           
            html += "<div class='content'>";
           
            // Device mode info
            html += "<div class='info-box'>";
            html += "<strong>Device Mode: " + deviceMode + "</strong>";
            if (deviceMode == "Transmitter") {
                html += "This device will read an ultrasonic sensor and transmit data via LoRa.";
            } else {
                html += "This device will receive LoRa data and forward it to MQTT/Home Assistant.";
            }
            html += "</div>";
           
            // Tabs
            html += "<div class='tabs'>";
            html += "<button class='tab active' onclick='showTab(\"tab-basic\")'>Basic</button>";
            html += "<button class='tab' onclick='showTab(\"tab-tank\")'>Tank</button>";
            html += "<button class='tab' onclick='showTab(\"tab-sensor\")'>Sensor</button>";
            html += "<button class='tab' onclick='showTab(\"tab-lora\")'>LoRa RF</button>";
            html += "<button class='tab' onclick='showTab(\"tab-network\")'>Network</button>";
            html += "</div>";
           
            html += "<form action='/save' method='POST' onsubmit='return validateForm()'>";
           
            // BASIC TAB
            html += "<div id='tab-basic' class='tab-content active'>";
            html += "<h3>Basic Settings</h3>";
            html += "<div class='input-group'>";
            html += "<label class='required'>Device Name</label>";
            html += "<input type='text' name='host' value='" + host + "' required>";
            html += "<div class='input-hint'>Used for WiFi hostname and MQTT client ID</div>";
            html += "</div>";
           
            if (deviceMode == "Transmitter") {
                html += "<div class='input-group'>";
                html += "<label class='required'>Transmitter ID (2-8 chars)</label>";
                html += "<input type='text' name='tx_id' placeholder='e.g., TANK1' maxlength='8' pattern='[A-Za-z0-9]{2,8}' required>";
                html += "<div class='input-hint'>Unique identifier for this transmitter (alphanumeric only)</div>";
                html += "</div>";
               
                html += "<div class='input-group'>";
                html += "<label>Display Name</label>";
                html += "<input type='text' name='tx_name' placeholder='e.g., Main Water Tank' maxlength='32'>";
                html += "<div class='input-hint'>Human-readable name shown on receiver display</div>";
                html += "</div>";
               
                html += "<div class='input-group'>";
                html += "<label>Transmission Interval (seconds)</label>";
                html += "<input type='number' name='tx_interval' value='30' min='10' max='300'>";
                html += "<div class='input-hint'>How often to send data (10-300 seconds)</div>";
                html += "</div>";
            }
            html += "</div>";
           
            // TANK TAB
            html += "<div id='tab-tank' class='tab-content'>";
            html += "<h3>Tank Calibration</h3>";
            html += "<p style='color: #666; margin: 10px 0;'><strong>Required:</strong> Measure sensor distance when tank is FULL and EMPTY</p>";
           
            html += "<div style='margin: 15px 0;'>";
            html += "<p style='font-size: 13px; color: #999; margin: 5px 0;'>Quick presets:</p>";
            html += "<div class='preset-group'>";
            html += "<button type='button' class='preset-btn' onclick='setTankPreset(30, 140)'>Standard Tank</button>";
            html += "<button type='button' class='preset-btn' onclick='setTankPreset(20, 120)'>Small Tank</button>";
            html += "<button type='button' class='preset-btn' onclick='setTankPreset(40, 200)'>Large Tank</button>";
            html += "</div>";
            html += "</div>";
           
            html += "<div class='input-group'>";
            html += "<label class='required'>Distance when FULL (cm)</label>";
            html += "<input type='number' step='0.1' name='distFull' value='30.0' min='2' max='200' required>";
            html += "<div class='input-hint'>Sensor distance to water surface when tank is full</div>";
            html += "</div>";
           
            html += "<div class='input-group'>";
            html += "<label class='required'>Distance when EMPTY (cm)</label>";
            html += "<input type='number' step='0.1' name='distEmpty' value='140.0' min='2' max='450' required>";
            html += "<div class='input-hint'>Sensor distance to tank bottom when empty</div>";
            html += "</div>";
           
            html += "<div id='calibration-status'></div>";
            html += "</div>";
           
            // SENSOR TAB
            html += "<div id='tab-sensor' class='tab-content'>";
            html += "<h3>Sensor Configuration</h3>";
           
            html += "<div class='input-group'>";
            html += "<label>Sensor Type</label>";
            html += "<select name='sensor_type'>";
            html += "<option value='0' selected>Auto-detect (Recommended)</option>";
            html += "<option value='1'>PWM Sensor (HC-SR04)</option>";
            html += "<option value='2'>Serial Sensor (A02YYUW)</option>";
            html += "</select>";
            html += "<div class='input-hint'>Auto-detect will test both sensor types during startup</div>";
            html += "</div>";
           
            html += "<div class='wiring-box'>";
            html += "<strong>Wiring Guide:</strong>";
            html += "<p><strong>PWM Sensor (HC-SR04):</strong><br>";
            html += "VCC &rarr; Vext | GND &rarr; GND | TRIG &rarr; GPIO5 | ECHO &rarr; GPIO6</p>";
            html += "<p><strong>Serial Sensor (A02YYUW):</strong><br>";
            html += "VCC &rarr; Vext | GND &rarr; GND | TX &rarr; GPIO5 | RX &rarr; GPIO6</p>";
            html += "<p style='margin-top: 10px; padding-top: 10px; border-top: 1px solid rgba(0,0,0,0.1); margin-bottom: 0;'>";
            html += "<strong>Important:</strong> Connect VCC to Vext pin, NOT 5V!</p>";
            html += "</div>";
            html += "</div>";
           
            // LoRa RF TAB
            html += "<div id='tab-lora' class='tab-content'>";
            html += "<h3>LoRa RF Settings</h3>";
            html += "<p style='color: #f44336; margin: 10px 0;'><strong>IMPORTANT:</strong> ";
            html += "All devices (TX and RX) MUST use identical LoRa settings!</p>";
           
            html += "<div style='margin: 15px 0;'>";
            html += "<p style='font-size: 13px; color: #999; margin: 5px 0;'>Configuration presets:</p>";
            html += "<div class='preset-group'>";
            html += "<button type='button' class='preset-btn-rf' onclick='setLoRaPreset(\"balanced\")'>Balanced</button>";
            html += "<button type='button' class='preset-btn-rf' onclick='setLoRaPreset(\"long_range\")'>Long Range</button>";
            html += "<button type='button' class='preset-btn-rf' onclick='setLoRaPreset(\"fast\")'>Fast</button>";
            html += "<button type='button' class='preset-btn-rf' onclick='setLoRaPreset(\"low_power\")'>Low Power</button>";
            html += "</div>";
            html += "</div>";
           
            html += "<div class='input-group'>";
            html += "<label class='required'>Frequency (MHz)</label>";
            html += "<input type='number' step='0.001' name='lora_freq' value='910.525' min='902' max='928' required>";
            html += "<div class='input-hint'>US ISM Band: 902-928 MHz | EU: 868 MHz</div>";
            html += "</div>";
           
            html += "<div class='input-group'>";
            html += "<label class='required'>Spreading Factor (SF)</label>";
            html += "<select name='lora_sf' required>";
            html += "<option value='7'>SF7 - Fastest, shortest range</option>";
            html += "<option value='8'>SF8</option>";
            html += "<option value='9' selected>SF9 - Balanced</option>";
            html += "<option value='10'>SF10</option>";
            html += "<option value='11'>SF11</option>";
            html += "<option value='12'>SF12 - Slowest, longest range</option>";
            html += "</select>";
            html += "<div class='input-hint'>Higher SF = Better range and SNR, but slower and more airtime</div>";
            html += "</div>";
           
            html += "<div class='input-group'>";
            html += "<label class='required'>Bandwidth (kHz)</label>";
            html += "<select name='lora_bw' required>";
            html += "<option value='125'>125 kHz - Best range</option>";
            html += "<option value='250' selected>250 kHz - Balanced</option>";
            html += "<option value='500'>500 kHz - Fastest</option>";
            html += "</select>";
            html += "<div class='input-hint'>Lower BW = Better range but slower speed</div>";
            html += "</div>";
           
            html += "<div class='input-group'>";
            html += "<label class='required'>Coding Rate</label>";
            html += "<select name='lora_cr' required>";
            html += "<option value='5'>4/5 - Less overhead</option>";
            html += "<option value='6'>4/6</option>";
            html += "<option value='7'>4/7</option>";
            html += "<option value='8' selected>4/8 - Most robust</option>";
            html += "</select>";
            html += "<div class='input-hint'>Higher CR = More error correction, more airtime</div>";
            html += "</div>";
           
            html += "<div class='input-group'>";
            html += "<label class='required'>Transmit Power (dBm)</label>";
            html += "<select name='lora_power' required>";
            for (int i = 0; i <= 15; i++) {
                html += "<option value='" + String(i) + "'";
                if (i == 1) html += " selected";
                html += ">" + String(i) + " dBm</option>";
            }
            html += "</select>";
            html += "<div class='input-hint'>Higher power = More range but more power consumption</div>";
            html += "</div>";
            html += "</div>";
           
            // NETWORK TAB
            html += "<div id='tab-network' class='tab-content'>";
           
            if (deviceMode == "Receiver") {
                html += "<h3>Network Settings</h3>";
                html += "<p style='color: #666;'>Required for MQTT connection to Home Assistant</p>";
               
                html += "<div class='section-divider'></div>";
                html += "<h4 style='margin-top: 0;'>WiFi Configuration</h4>";
               
                html += "<div class='input-group'>";
                html += "<label class='required'>WiFi SSID</label>";
                html += "<input type='text' name='ssid' placeholder='Your WiFi network name' required>";
                html += "</div>";
               
                html += "<div class='input-group'>";
                html += "<label>WiFi Password</label>";
                html += "<div class='password-wrapper'>";
                html += "<input type='password' id='wifi_pass' name='pass' placeholder='Leave blank if open network'>";
                html += "<button type='button' class='password-toggle' onclick='togglePasswordVisibility(\"wifi_pass\", this)'>Show</button>";
                html += "</div>";
                html += "</div>";
               
                html += "<div class='section-divider'></div>";
                html += "<h4 style='margin-top: 0;'>MQTT Server</h4>";
               
                html += "<div class='input-group'>";
                html += "<label class='required'>MQTT Server Address</label>";
                html += "<input type='text' name='mqtt_server' value='192.168.1.132' placeholder='e.g., 192.168.1.132 or mqtt.example.com' required>";
                html += "<div class='input-hint'>IP address or hostname of your MQTT broker</div>";
                html += "</div>";
               
                html += "<div class='input-group'>";
                html += "<label class='required'>MQTT Port</label>";
                html += "<input type='number' name='mqtt_port' value='1883' min='1' max='65535' required>";
                html += "<div class='input-hint'>Default: 1883 (non-TLS), 8883 (TLS)</div>";
                html += "</div>";
               
                html += "<div class='input-group'>";
                html += "<label>MQTT Username (optional)</label>";
                html += "<input type='text' name='mqtt_user' placeholder='Leave blank if no authentication'>";
                html += "</div>";
               
                html += "<div class='input-group'>";
                html += "<label>MQTT Password</label>";
                html += "<div class='password-wrapper'>";
                html += "<input type='password' id='mqtt_pass' name='mqtt_pass' placeholder='Leave blank if no authentication'>";
                html += "<button type='button' class='password-toggle' onclick='togglePasswordVisibility(\"mqtt_pass\", this)'>Show</button>";
                html += "</div>";
                html += "</div>";
               
                html += "<div class='section-divider'></div>";
                html += "<h4 style='margin-top: 0;'>Home Assistant Discovery</h4>";
               
                html += "<div class='input-group'>";
                html += "<label>Discovery Prefix</label>";
                html += "<input type='text' name='ha_discovery_prefix' value='homeassistant'>";
                html += "<div class='input-hint'>Default: homeassistant (only change if customized in Home Assistant)</div>";
                html += "</div>";
               
            } else {
                html += "<h3>WiFi Settings (Optional)</h3>";
                html += "<p style='color: #f44336;'><strong>Note:</strong> Transmitter will DISABLE WiFi after configuration to prevent interference with LoRa.</p>";
               
                html += "<div class='input-group'>";
                html += "<label>WiFi SSID (optional)</label>";
                html += "<input type='text' name='ssid' placeholder='Leave blank to disable WiFi'>";
                html += "<div class='input-hint'>Only needed if you want to reconfigure remotely later</div>";
                html += "</div>";
               
                html += "<div class='input-group'>";
                html += "<label>WiFi Password</label>";
                html += "<div class='password-wrapper'>";
                html += "<input type='password' id='wifi_pass' name='pass' placeholder='Leave blank if no password'>";
                html += "<button type='button' class='password-toggle' onclick='togglePasswordVisibility(\"wifi_pass\", this)'>Show</button>";
                html += "</div>";
                html += "</div>";
            }
            html += "</div>";
           
            html += "<div class='button-group'>";
            html += "<button type='submit'>Save Configuration &amp; Restart</button>";
            html += "</div>";
            html += "</form>";
           
            html += "</div></div>"; // content + container
            html += "</body></html>";
           
            webServer.send(200, "text/html; charset=UTF-8", html);
        });
       
        webServer.on("/save", HTTP_POST, []() {
            // Get all form parameters
            String ssid = webServer.arg("ssid");
            String pass = webServer.arg("pass");
            String host = webServer.arg("host");
            String distFull = webServer.arg("distFull");
            String distEmpty = webServer.arg("distEmpty");
            String tx_id = webServer.arg("tx_id");
            String tx_name = webServer.arg("tx_name");
            String tx_interval = webServer.arg("tx_interval");
            String sensor_type = webServer.arg("sensor_type");
            String mqtt_server = webServer.arg("mqtt_server");
            String mqtt_port = webServer.arg("mqtt_port");
            String mqtt_user = webServer.arg("mqtt_user");
            String mqtt_pass = webServer.arg("mqtt_pass");
            String ha_discovery_prefix = webServer.arg("ha_discovery_prefix");
           
            // LoRa RF Settings
            String lora_freq = webServer.arg("lora_freq");
            String lora_sf = webServer.arg("lora_sf");
            String lora_bw = webServer.arg("lora_bw");
            String lora_cr = webServer.arg("lora_cr");
            String lora_power = webServer.arg("lora_power");
           
            // VALIDATION - CRITICAL
            // Validate tank calibration
            if (distFull.isEmpty() || distEmpty.isEmpty()) {
                String errorHtml = "<!DOCTYPE html><html><body>";
                errorHtml += "<h2>Configuration Error</h2>";
                errorHtml += "<p>Tank distances are required!</p>";
                errorHtml += "<a href='/'>Back</a></body></html>";
                webServer.send(400, "text/html", errorHtml);
                return;
            }
           
            float fullDist = distFull.toFloat();
            float emptyDist = distEmpty.toFloat();
           
            if (fullDist >= emptyDist) {
                String errorHtml = "<!DOCTYPE html><html><body>";
                errorHtml += "<h2>Configuration Error</h2>";
                errorHtml += "<p>FULL distance (" + distFull + "cm) must be LESS than EMPTY distance (" + distEmpty + "cm)</p>";
                errorHtml += "<a href='/'>Back</a></body></html>";
                webServer.send(400, "text/html", errorHtml);
                return;
            }
           
            if (fullDist < 2 || emptyDist > 450) {
                String errorHtml = "<!DOCTYPE html><html><body>";
                errorHtml += "<h2>Configuration Error</h2>";
                errorHtml += "<p>Tank distances out of valid range (2-450cm)</p>";
                errorHtml += "<a href='/'>Back</a></body></html>";
                webServer.send(400, "text/html", errorHtml);
                return;
            }
           
            // Validate transmitter settings if in TX mode
            if (deviceMode == "Transmitter" && tx_id.isEmpty()) {
                String errorHtml = "<!DOCTYPE html><html><body>";
                errorHtml += "<h2>Configuration Error</h2>";
                errorHtml += "<p>Transmitter ID is required!</p>";
                errorHtml += "<a href='/'>Back</a></body></html>";
                webServer.send(400, "text/html", errorHtml);
                return;
            }
           
            // Validate LoRa settings
            float loraFreq = lora_freq.toFloat();
            if (loraFreq < 902 || loraFreq > 928) {
                String errorHtml = "<!DOCTYPE html><html><body>";
                errorHtml += "<h2>Configuration Error</h2>";
                errorHtml += "<p>LoRa frequency must be between 902-928 MHz</p>";
                errorHtml += "<a href='/'>Back</a></body></html>";
                webServer.send(400, "text/html", errorHtml);
                return;
            }
           
            // Validate receiver settings if in RX mode
            if (deviceMode == "Receiver" && ssid.isEmpty()) {
                String errorHtml = "<!DOCTYPE html><html><body>";
                errorHtml += "<h2>Configuration Error</h2>";
                errorHtml += "<p>WiFi SSID is required for Receiver mode!</p>";
                errorHtml += "<a href='/'>Back</a></body></html>";
                webServer.send(400, "text/html", errorHtml);
                return;
            }
           
            // If all validations pass, save to preferences
            preferences.begin("device-config", false);
           
            // Save calibration (required for both)
            preferences.putFloat("distFull", fullDist);
            preferences.putFloat("distEmpty", emptyDist);
            preferences.putString("host", host);
           
            // Save transmitter settings if provided
            if (deviceMode == "Transmitter") {
                preferences.putString("tx_id", tx_id);
                preferences.putString("tx_name", tx_name);
                if (!tx_interval.isEmpty()) {
                    preferences.putULong("tx_interval", tx_interval.toInt() * 1000);
                }
            }
           
            // Save sensor type preference
            if (!sensor_type.isEmpty()) {
                preferences.putInt("sensor_type", sensor_type.toInt());
            }
           
            // Save WiFi settings
            if (!ssid.isEmpty()) {
                preferences.putString("ssid", ssid);
                preferences.putString("pass", pass);
            }
           
            // Save MQTT settings only for receiver
            if (deviceMode == "Receiver") {
                preferences.putString("mqtt_server", mqtt_server);
                preferences.putInt("mqtt_port", mqtt_port.toInt());
                preferences.putString("mqtt_user", mqtt_user);
                preferences.putString("mqtt_pass", mqtt_pass);
                preferences.putString("ha_prefix", ha_discovery_prefix);
            }
           
            // Save LoRa RF settings (both devices)
            preferences.putFloat("lora_freq", loraFreq);
            preferences.putInt("lora_sf", lora_sf.toInt());
            preferences.putFloat("lora_bw", lora_bw.toFloat());
            preferences.putInt("lora_cr", lora_cr.toInt());
            preferences.putInt("lora_power", lora_power.toInt());
           
            // Set critical flags
            preferences.putBool("has_calibration", true);
            preferences.putBool("has_run_before", true);
            preferences.putString("sketch_version", CURRENT_SKETCH_VERSION);
           
            preferences.end();
           
            // Send success response
            String html = "<!DOCTYPE html><html><head>";
            html += "<meta charset='UTF-8'>";
            html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
            html += "<title>Configuration Saved</title>";
            html += "<style>";
            html += "body { font-family: Arial, sans-serif; background: #f5f5f5; margin: 0; padding: 20px; }";
            html += ".container { max-width: 500px; margin: 0 auto; background: white; border-radius: 8px; padding: 40px; text-align: center; }";
            html += ".success { color: #4caf50; font-size: 48px; margin-bottom: 20px; }";
            html += "h1 { color: #333; margin: 20px 0; }";
            html += ".summary { text-align: left; background: #f9f9f9; padding: 20px; border-radius: 6px; margin: 20px 0; }";
            html += ".summary-item { margin: 8px 0; padding: 8px 0; border-bottom: 1px solid #eee; }";
            html += ".summary-item:last-child { border-bottom: none; }";
            html += ".label { font-weight: 600; color: #333; display: inline-block; width: 120px; }";
            html += ".value { color: #666; }";
            html += ".loading { margin-top: 20px; }";
            html += "</style>";
            html += "</head><body>";
            html += "<div class='container'>";
            html += "<div class='success'>✓</div>";
            html += "<h1>Configuration Saved!</h1>";
           
            // Summary
            html += "<div class='summary'>";
            html += "<div class='summary-item'><span class='label'>Mode:</span> <span class='value'>" + deviceMode + "</span></div>";
            html += "<div class='summary-item'><span class='label'>Device:</span> <span class='value'>" + host + "</span></div>";
            html += "<div class='summary-item'><span class='label'>Tank Full:</span> <span class='value'>" + distFull + " cm</span></div>";
            html += "<div class='summary-item'><span class='label'>Tank Empty:</span> <span class='value'>" + distEmpty + " cm</span></div>";
           
            if (deviceMode == "Transmitter" && !tx_id.isEmpty()) {
                html += "<div class='summary-item'><span class='label'>TX ID:</span> <span class='value'>" + tx_id + "</span></div>";
            }
           
            html += "<div class='summary-item'><span class='label'>LoRa Freq:</span> <span class='value'>" + lora_freq + " MHz</span></div>";
            html += "<div class='summary-item'><span class='label'>Spreading:</span> <span class='value'>SF" + lora_sf + "</span></div>";
           
            if (deviceMode == "Receiver" && !mqtt_server.isEmpty()) {
                html += "<div class='summary-item'><span class='label'>MQTT:</span> <span class='value'>" + mqtt_server + ":" + mqtt_port + "</span></div>";
            }
           
            html += "</div>";
           
            html += "<div class='loading'><p>Device will restart in 5 seconds...</p>";
            html += "<p style='color: #999; font-size: 12px;'>Please wait...</p></div>";
            html += "</div>";
            html += "</body></html>";
           
            webServer.send(200, "text/html; charset=UTF-8", html);
           
            // Update display
            display.clear();
            display.drawString(0, 0, "CONFIG SAVED");
            display.drawString(0, 15, "Restarting...");
            display.display();
           
            delay(5000);
            ESP.restart();
        });
       
        webServer.onNotFound([]() {
            webServer.sendHeader("Location", "http://192.168.4.1/", true);
            webServer.send(302, "text/plain", "");
        });
       
        webServer.begin();
        Serial.println("Web server started");
       
        portalActive = true;
        portalStartTime = millis();
        currentState = STATE_PORTAL;
       
        display.clear();
        display.drawString(0, 0, "CONFIG PORTAL");
        display.drawString(0, 15, "SSID: " + apName);
        display.drawString(0, 25, "IP: 192.168.4.1");
        display.drawString(0, 35, "Password: none");
        display.drawString(0, 45, "Open browser...");
        display.display();
       
    } else {
        Serial.println("ERROR: Failed to start Access Point!");
        display.clear();
        display.drawString(0, 0, "PORTAL ERROR");
        display.drawString(0, 15, "Failed to start");
        display.drawString(0, 25, "WiFi AP");
        display.display();
       
        portalActive = false;
        currentState = STATE_ERROR;
    }
}

// ============================================================================
// MQTT FUNCTIONS (Receiver only)
// ============================================================================

void reconnect() {
    if (deviceMode != "Receiver") return;
   
    int retryCount = 0;
    while (!client.connected() && retryCount < 5) {
        Serial.print("Attempting MQTT connection...");
       
        String clientId = "TankReceiver-" + String(random(0xffff), HEX);
       
        client.setBufferSize(1024);
       
        bool connected = false;
        if (mqtt_user != "" && mqtt_pass != "") {
            connected = client.connect(clientId.c_str(), mqtt_user.c_str(), mqtt_pass.c_str());
        } else {
            connected = client.connect(clientId.c_str());
        }
       
        if (connected) {
            Serial.println("MQTT connected!");
            currentState = STATE_CONNECTED;
           
            client.subscribe("homeassistant/status");
           
            // Send discovery for existing transmitters
            for (const auto& tx : transmitters) {
                sendHADiscovery(tx.first, tx.second.name);
            }
            return;
        } else {
            Serial.print("MQTT failed, rc=");
            Serial.print(client.state());
            Serial.println(" retrying...");
            retryCount++;
            delay(5000);
        }
    }
   
    Serial.println("Failed to connect to MQTT");
    currentState = STATE_ERROR;
}

void sendHADiscovery(String txId, String displayName) {
    if (deviceMode != "Receiver" || !client.connected()) return;
   
    Serial.printf("Sending HA discovery for transmitter %s...\n", txId.c_str());
   
    device_identifier = String((uint32_t)ESP.getEfuseMac(), HEX);
    ha_discovery_prefix = preferences.getString("ha_prefix", "homeassistant");
   
    String safe_tx_id = txId;
    safe_tx_id.replace("-", "_");
    safe_tx_id.replace(" ", "_");
   
    String base_topic = "tank_monitor/" + safe_tx_id;
   
    // Distance sensor
    String dist_id = device_identifier.substring(0, 8) + "_" + safe_tx_id + "_dist";
    String dist_topic = ha_discovery_prefix + "/sensor/" + dist_id + "/config";
   
    String dist_config = "{";
    dist_config += "\"name\":\"" + displayName + " Distance\",";
    dist_config += "\"uniq_id\":\"" + dist_id + "\",";
    dist_config += "\"stat_t\":\"" + base_topic + "/distance\",";
    dist_config += "\"unit_of_meas\":\"cm\",";
    dist_config += "\"dev_cla\":\"distance\",";
    dist_config += "\"dev\":{\"ids\":[\"" + device_identifier.substring(0, 8) + "_" + safe_tx_id + "\"],";
    dist_config += "\"name\":\"" + displayName + "\",";
    dist_config += "\"mdl\":\"" + device_model + "\"}";
    dist_config += "}";
   
    client.publish(dist_topic.c_str(), dist_config.c_str(), true);
    delay(50);
   
    // Percentage sensor
    String pct_id = device_identifier.substring(0, 8) + "_" + safe_tx_id + "_pct";
    String pct_topic = ha_discovery_prefix + "/sensor/" + pct_id + "/config";
   
    String pct_config = "{";
    pct_config += "\"name\":\"" + displayName + " Fill Level\",";
    pct_config += "\"uniq_id\":\"" + pct_id + "\",";
    pct_config += "\"stat_t\":\"" + base_topic + "/percent\",";
    pct_config += "\"unit_of_meas\":\"%\",";
    pct_config += "\"dev_cla\":\"battery\",";
    pct_config += "\"ic\":\"mdi:water-percent\"";
    pct_config += "}";
   
    client.publish(pct_topic.c_str(), pct_config.c_str(), true);
    delay(50);
   
    Serial.printf("Discovery sent for transmitter %s!\n", txId.c_str());
}

void publishSensorData(String txId, String displayName, float distance, float percent, float rssi, float snr) {
    if (deviceMode != "Receiver" || !client.connected()) return;
   
    String safe_tx_id = txId;
    safe_tx_id.replace("-", "_");
    safe_tx_id.replace(" ", "_");
   
    String base_topic = "tank_monitor/" + safe_tx_id;
   
    char payload[32];
   
    // Distance
    String dist_topic = base_topic + "/distance";
    snprintf(payload, sizeof(payload), "%.2f", distance);
    client.publish(dist_topic.c_str(), payload, true);
   
    // Percentage
    String pct_topic = base_topic + "/percent";
    snprintf(payload, sizeof(payload), "%.1f", percent);
    client.publish(pct_topic.c_str(), payload, true);
   
    // RSSI
    String rssi_topic = base_topic + "/rssi";
    snprintf(payload, sizeof(payload), "%.2f", rssi);
    client.publish(rssi_topic.c_str(), payload, true);
   
    // SNR
    String snr_topic = base_topic + "/snr";
    snprintf(payload, sizeof(payload), "%.2f", snr);
    client.publish(snr_topic.c_str(), payload, true);
   
    // Last seen timestamp
    String time_topic = base_topic + "/last_seen";
    snprintf(payload, sizeof(payload), "%lu", millis() / 1000);
    client.publish(time_topic.c_str(), payload, true);
}

void setupMQTTDebug() {
    client.setCallback([](char* topic, byte* payload, unsigned int length) {
        Serial.print("MQTT Message [");
        Serial.print(topic);
        Serial.print("]: ");
        for (int i = 0; i < length; i++) {
            Serial.print((char)payload[i]);
        }
        Serial.println();
       
        if (String(topic) == "homeassistant/status") {
            String message = "";
            for (int i = 0; i < length; i++) {
                message += (char)payload[i];
            }
            if (message == "online") {
                Serial.println("HA online - resending discovery");
                // Resend discovery for all transmitters
                if (deviceMode == "Receiver") {
                    for (const auto& tx : transmitters) {
                        sendHADiscovery(tx.first, tx.second.name);
                    }
                }
            }
        }
    });
}

// ============================================================================
// LoRa FUNCTIONS
// ============================================================================

/**
 * @brief Prepare LoRa payload with transmitter ID and distance
 */
String prepareLoRaPayload(float distance) {
    char payload[64];
    // Enhanced payload format: "ID|NAME|DISTANCE|TIMESTAMP|SENSOR_TYPE"
    unsigned long timestamp = millis() / 1000;
    snprintf(payload, sizeof(payload), "%s|%s|%.2f|%lu|%s",
             transmitterID.c_str(), transmitterName.c_str(), distance, timestamp,
             sensorType == SENSOR_PWM ? "PWM" : "Serial");
    return String(payload);
}

/**
 * @brief Parse LoRa payload containing transmitter ID and distance
 */
bool parseLoRaPayload(String payload, String& txId, String& txName, float& distance, unsigned long& timestamp, String& sensorTypeStr) {
    // Expected format: "ID|NAME|DISTANCE|TIMESTAMP|SENSOR_TYPE"
    std::vector<String> parts;
    int startIndex = 0;
    int separatorIndex;
   
    while ((separatorIndex = payload.indexOf('|', startIndex)) != -1) {
        parts.push_back(payload.substring(startIndex, separatorIndex));
        startIndex = separatorIndex + 1;
    }
    parts.push_back(payload.substring(startIndex));
   
    if (parts.size() >= 5) {
        txId = parts[0];
        txName = parts[1];
        distance = parts[2].toFloat();
        timestamp = parts[3].toInt();
        sensorTypeStr = parts[4];
        return true;
    } else if (parts.size() >= 4) {
        // Older format: "ID|NAME|DISTANCE|TIMESTAMP"
        txId = parts[0];
        txName = parts[1];
        distance = parts[2].toFloat();
        timestamp = parts[3].toInt();
        sensorTypeStr = "Unknown";
        return true;
    } else if (parts.size() >= 2) {
        // Backward compatibility: "ID|DISTANCE"
        txId = parts[0];
        txName = "Tank " + txId;
        distance = parts[1].toFloat();
        timestamp = millis() / 1000;
        sensorTypeStr = "Unknown";
        return true;
    }
   
    Serial.println("Invalid payload format");
    return false;
}

void processLoRaPacket() {
    if (!rxFlag) return;
   
    rxFlag = false;
    int state = radio.readData(rxdata);
    if (state == RADIOLIB_ERR_NONE) {
        Serial.printf("RX [%s]\n", rxdata.c_str());
       
        String txId, txName, sensorTypeStr;
        float distance;
        unsigned long timestamp;
       
        if (parseLoRaPayload(rxdata, txId, txName, distance, timestamp, sensorTypeStr)) {
            float rssi = radio.getRSSI();
            float snr = radio.getSNR();
            Serial.printf("  From: %s (%s)\n", txId.c_str(), txName.c_str());
            Serial.printf("  Distance: %.2f cm\n", distance);
            Serial.printf("  RSSI: %.2f dBm, SNR: %.2f dB\n", rssi, snr);
            Serial.printf("  Sensor Type: %s\n", sensorTypeStr.c_str());
            Serial.printf("  Age: %lu seconds\n", (millis() / 1000) - timestamp);
           
            // Receiver processing
            if (deviceMode == "Receiver") {
                unsigned long now = millis();
               
                // Check if this is a new transmitter
                bool isNewTransmitter = transmitters.find(txId) == transmitters.end();
               
                // Calculate percentage
                float percent = 0.0;
                if (hasCalibration) {
                    percent = 100.0 * (minFull - distance) / (minFull - maxFull);
                    percent = constrain(percent, 0.0, 100.0);
                }
               
                // Create or update transmitter data
                TransmitterData data;
                data.id = txId;
                data.name = txName;
                data.distance = String(distance, 1);
                data.percentFull = String(percent, 1);
                data.rssi = rssi;
                data.snr = snr;
                data.lastSeen = now;
                data.sensorType = sensorTypeStr; // ADDED: Store sensor type
                data.packetCount = (transmitters.count(txId) > 0) ? transmitters[txId].packetCount + 1 : 1;
               
                transmitters[txId] = data;
                totalPacketsReceived++;
               
                // If this is a new transmitter, send HA discovery
                if (isNewTransmitter && client.connected()) {
                    sendHADiscovery(txId, txName);
                }
               
                // Publish to MQTT
                if (client.connected()) {
                    publishSensorData(txId, txName, distance, percent, rssi, snr);
                }
               
                // Update display cycle if needed
                if (currentDisplayTransmitter == "" || isNewTransmitter) {
                    currentDisplayTransmitter = txId;
                }
            }
           
        } else {
            Serial.println("Failed to parse payload");
            errorCount++;
        }
    } else {
        Serial.printf("RX error: %d\n", state);
        errorCount++;
    }
    RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF));
}

// ============================================================================
// SETUP FUNCTION - FIXED VERSION
// ============================================================================

void setup() {
    // Initialize display FIRST, before heltec_setup
    initDisplay();
   
    // Now call heltec_setup
    heltec_setup();
   
    Serial.begin(115200);
    delay(2000);
   
    // Store display pointer for later use
    displayPtr = &display;
   
    Serial.println("\n\n==========================================");
    Serial.println("   Enhanced Tank Monitor v" + CURRENT_SKETCH_VERSION);
    Serial.println("==========================================");
   
    // DEBUG: Check all preferences
    preferences.begin("device-config", false);
    Serial.println("=== PREFERENCES DUMP ===");
    Serial.printf("has_run_before: %d\n", preferences.getBool("has_run_before", false));
    Serial.printf("has_calibration: %d\n", preferences.getBool("has_calibration", false));
    Serial.printf("sketch_version: %s\n", preferences.getString("sketch_version", "").c_str());
    Serial.printf("sensor_type: %d\n", preferences.getInt("sensor_type", 0)); // ADDED: Show sensor type
    Serial.printf("host: %s\n", preferences.getString("host", "").c_str());
    Serial.printf("distFull: %.1f\n", preferences.getFloat("distFull", 0));
    Serial.printf("distEmpty: %.1f\n", preferences.getFloat("distEmpty", 0));
    preferences.end();
    Serial.println("=== END DUMP ===");
   
    // TEMPORARY DEBUG: Test sensor directly before mode detection
    Serial.println("\n=== DIRECT SENSOR DEBUG TEST ===");
    Serial.println("Powering on sensor (Vext LOW)...");
    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, LOW);
    delay(1000);
   
    Serial.println("Testing sensors on GPIO5/6...");
    Serial.printf("PWM: TRIG→GPIO%d, ECHO→GPIO%d\n", TRIGGER_PIN, ECHO_PIN);
    Serial.printf("Serial: TX→GPIO%d, RX→GPIO%d\n", TRIGGER_PIN, ECHO_PIN);
   
    // Take a few test readings
    Serial.println("Taking test readings...");
    for (int i = 0; i < 5; i++) {
        unsigned int pwmDist = sonar.ping_cm();
        Serial1.begin(9600, SERIAL_8N1, 5, 6);
        delay(50);
        serialSensor.update();
        float serialDist = serialSensor.getDistance();
        Serial.printf("Test %d: PWM=%u cm, Serial=%.1f cm\n", i + 1, pwmDist, serialDist);
        delay(500);
    }
   
    // Check if we need to force configuration portal (version change)
    bool forcePortal = isFirstRunAfterUpload();
   
    // DETERMINE DEVICE MODE FIRST - This is critical!
    deviceMode = determineDeviceMode();
    Serial.println("Device mode determined: " + deviceMode);
   
    if (forcePortal) {
        Serial.println("\n⚠️ SKETCH VERSION CHANGE DETECTED");
        Serial.println("Forcing configuration portal...");
       
        // Force configuration portal for new version
        forceConfigurationPortal();
        return; // Stop here and show portal
    }
   
    // Initialize preferences
    preferences.begin("device-config", false);
   
    // Check if we have calibration data
    hasCalibration = preferences.getBool("has_calibration", false);
   
    if (!hasCalibration) {
        Serial.println("\n⚠️ NO CALIBRATION DATA FOUND");
        Serial.println("Starting configuration portal...");
       
        display.clear();
        display.drawString(0, 0, "NO CALIBRATION");
        display.drawString(0, 15, "Mode: " + deviceMode);
        display.drawString(0, 30, "Starting Portal...");
        display.display();
        delay(2000);
       
        preferences.end();
        startConfigurationPortal();
        return; // Stop here until configured
    }
   
    // Load calibration (both transmitter and receiver need this)
    minFull = preferences.getFloat("distEmpty", 140.0);
    maxFull = preferences.getFloat("distFull", 30.0);
   
    Serial.printf("Calibration loaded: Full=%.1fcm, Empty=%.1fcm\n", maxFull, minFull);
   
    // Load transmitter ID if this is a transmitter
    if (deviceMode == "Transmitter") {
        transmitterID = preferences.getString("tx_id", "");
        transmitterName = preferences.getString("tx_name", "");
        transmitInterval = preferences.getULong("tx_interval", 30000);
       
        if (transmitterID == "") {
            // Generate a default ID based on MAC address
            uint32_t chipId = (uint32_t)ESP.getEfuseMac();
            transmitterID = "TX" + String(chipId & 0xFF, HEX);
            preferences.putString("tx_id", transmitterID);
        }
        Serial.printf("Transmitter ID: %s\n", transmitterID.c_str());
        Serial.printf("Transmitter Name: %s\n", transmitterName.c_str());
        Serial.printf("Transmit interval: %lu ms\n", transmitInterval);
       
        // ADDED: Load sensor type preference if transmitter
        int prefSensorType = preferences.getInt("sensor_type", 0);
        if (prefSensorType == 1) {
            sensorType = SENSOR_PWM;
            Serial.println("Sensor type: PWM (from preferences)");
        } else if (prefSensorType == 2) {
            sensorType = SENSOR_SERIAL;
            Serial.println("Sensor type: Serial (from preferences)");
        } else {
            // Auto-detect already done in determineDeviceMode()
            Serial.printf("Sensor type: %s (auto-detected)\n",
                         sensorType == SENSOR_PWM ? "PWM" :
                         sensorType == SENSOR_SERIAL ? "Serial" : "None");
        }
    }
   
    // Load LoRa RF settings (both transmitter and receiver)
    FREQUENCY = preferences.getFloat("lora_freq", 910.525);
    SPREADING_FACTOR = preferences.getInt("lora_sf", 9);
    BANDWIDTH = preferences.getFloat("lora_bw", 250.0);
    CODING_RATE = preferences.getInt("lora_cr", 8);
    TRANSMIT_POWER = preferences.getInt("lora_power", 1);
   
    Serial.printf("LoRa Settings: %.3f MHz, SF%d, BW%.0f kHz, CR4/%d, %d dBm\n",
                  FREQUENCY, SPREADING_FACTOR, BANDWIDTH, CODING_RATE, TRANSMIT_POWER);
   
    // For TRANSMITTER: DISABLE WiFi completely to prevent interference
    if (deviceMode == "Transmitter") {
        Serial.println("\nTransmitter mode: DISABLING WiFi to prevent display interference");
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        Serial.println("WiFi disabled - display should be stable now");
    }
    // For RECEIVER: Only proceed with WiFi/MQTT if we have WiFi credentials
    else if (deviceMode == "Receiver") {
        String ssid = preferences.getString("ssid", "");
        String pass = preferences.getString("pass", "");
        device_name = preferences.getString("host", "Tank-Monitor");
       
        if (ssid != "") {
            Serial.println("\nAttempting WiFi connection as receiver...");
           
            WiFi.disconnect(true);
            delay(100);
            WiFi.mode(WIFI_STA);
            delay(100);
            WiFi.setHostname(device_name.c_str());
            WiFi.begin(ssid.c_str(), pass.c_str());
           
            Serial.print("Connecting");
            unsigned long start = millis();
            while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
                delay(500);
                Serial.print(".");
            }
           
            if (WiFi.status() == WL_CONNECTED) {
                Serial.println("\n✅ WiFi Connected!");
                Serial.println("IP Address: " + WiFi.localIP().toString());
               
                MDNS.begin(device_name.c_str());
                currentState = STATE_CONNECTED;
               
                // Load MQTT settings for receiver
                mqtt_server = preferences.getString("mqtt_server", "192.168.1.132");
                mqtt_port = preferences.getInt("mqtt_port", 1883);
                mqtt_user = preferences.getString("mqtt_user", "");
                mqtt_pass = preferences.getString("mqtt_pass", "");
                ha_discovery_prefix = preferences.getString("ha_prefix", "homeassistant");
               
                // Setup MQTT
                client.setServer(mqtt_server.c_str(), mqtt_port);
                setupMQTTDebug();
                reconnect();
               
            } else {
                Serial.println("\n❌ WiFi connection failed!");
                // Receiver without WiFi can still receive LoRa, just no MQTT
            }
        } else {
            Serial.println("\n⚠️ Receiver mode but no WiFi configured");
            Serial.println("Can receive LoRa but won't send to MQTT");
        }
    }
   
    preferences.end();
   
    // Set display font
    display.setFont(ArialMT_Plain_10);
   
    // Initialize LoRa (both transmitter and receiver)
    RADIOLIB_OR_HALT(radio.begin());
    radio.setDio1Action(rx);
    RADIOLIB_OR_HALT(radio.setFrequency(FREQUENCY));
    RADIOLIB_OR_HALT(radio.setBandwidth(BANDWIDTH));
    RADIOLIB_OR_HALT(radio.setSpreadingFactor(SPREADING_FACTOR));
    RADIOLIB_OR_HALT(radio.setCodingRate(CODING_RATE));
    RADIOLIB_OR_HALT(radio.setOutputPower(TRANSMIT_POWER));
    RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF));
   
    Serial.println("\n=== SETUP COMPLETE ===");
    Serial.println("Mode: " + deviceMode);
    if (deviceMode == "Transmitter") {
        Serial.printf("Transmitter ID: %s\n", transmitterID.c_str());
        Serial.printf("Transmitter Name: %s\n", transmitterName.c_str());
        Serial.printf("Sensor type: %s\n",
                     sensorType == SENSOR_PWM ? "PWM" :
                     sensorType == SENSOR_SERIAL ? "Serial" : "None");
        Serial.println("WiFi: DISABLED");
        Serial.println("Sensor: ENABLED");
    } else if (deviceMode == "Receiver") {
        Serial.println("WiFi: " + String(WiFi.status() == WL_CONNECTED ? "CONNECTED" : "DISCONNECTED"));
        Serial.println("MQTT: READY");
        Serial.println("MQTT Server: " + mqtt_server + ":" + String(mqtt_port));
        if (mqtt_user != "") {
            Serial.println("MQTT Authentication: Enabled");
        }
        Serial.println("Sensor: DISABLED (receiver mode)");
    }
    Serial.println("Has calibration: " + String(hasCalibration ? "Yes" : "No"));
   
    bootTime = millis();
}

// ============================================================================
// MAIN LOOP - OPTIMIZED
// ============================================================================

void rx() {
    rxFlag = true;
}

void loop() {
    heltec_loop();
   
    // Check for factory reset button press
    checkFactoryReset();
   
    // Handle portal if active (for configuration)
    if (portalActive) {
        dnsServer.processNextRequest();
        webServer.handleClient();
       
        if (millis() - portalStartTime > PORTAL_TIMEOUT) {
            Serial.println("Portal timeout - restarting");
            ESP.restart();
        }
       
        return;
    }
   
    // Process LoRa packets (both transmitter and receiver)
    if (rxFlag) {
        processLoRaPacket();
    }
   
    // Update sensor reading at fixed interval (transmitter only)
    updateSensorReading();
   
    // Cycle through transmitters on display (receiver only)
    cycleDisplayTransmitter();
   
    // Clean up old transmitters (receiver only)
    cleanupOldTransmitters();
   
    // Update display at fixed interval (always on, 1 second refresh)
    updateDisplay();
   
    // TRANSMITTER SPECIFIC LOGIC
    if (deviceMode == "Transmitter") {
        // Transmit via LoRa
        bool tx_legal = millis() > last_tx + minimum_pause;
       
        if ((transmitInterval && tx_legal && millis() - last_tx > transmitInterval) || button.isSingleClick()) {
            if (!tx_legal) {
                int waitSeconds = (int)((minimum_pause - (millis() - last_tx)) / 1000) + 1;
                Serial.printf("Wait %i sec (duty cycle)\n", waitSeconds);
                txString = "Wait " + String(waitSeconds) + "s";
            } else if (sensorReadingValid) {
                // Start transmission using cached sensor value
                currentState = STATE_TRANSMITTING;
               
                radio.clearDio1Action();
                heltec_led(50);
               
                String payload = prepareLoRaPayload(lastValidDistance);
               
                Serial.println("Transmitting: " + payload);
                unsigned long startTime = millis();
                int state = radio.transmit(payload.c_str());
                heltec_led(0);
               
                if (state == RADIOLIB_ERR_NONE) {
                    tx_time = millis() - startTime;
                    Serial.printf("TX OK (%i ms)\n", (int)tx_time);
                   
                    minimum_pause = tx_time * 99;
                    last_tx = millis();
                    lastTransmitTime = millis();
                    totalPacketsTransmitted++;
                    txString = "TX: " + String(lastValidDistance, 1) + "cm";
                } else {
                    Serial.printf("TX fail (%i)\n", state);
                    txString = "TX Failed";
                    errorCount++;
                }
               
                radio.setDio1Action(rx);
                RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF));
               
                currentState = STATE_CONNECTED;
            }
        }
    }
    // RECEIVER SPECIFIC LOGIC
    else if (deviceMode == "Receiver") {
        // MQTT loop for receiver only
        if (WiFi.status() == WL_CONNECTED) {
            if (!client.connected()) {
                reconnect();
            }
            client.loop();
           
            // Publish receiver status every 60 seconds
            unsigned long now = millis();
            if (now - lastMsg > 60000) {
                lastMsg = now;
                String payload = "{\"device\": \"receiver\",";
                payload += "\"ip\": \"" + WiFi.localIP().toString() + "\",";
                payload += "\"rssi\": " + String(WiFi.RSSI()) + ",";
                payload += "\"uptime\": " + String(now / 1000) + ",";
                payload += "\"packets\": " + String(totalPacketsReceived) + ",";
                payload += "\"transmitters\": " + String(transmitters.size()) + "}";
                client.publish("tank_monitor/receiver/status", payload.c_str());
            }
        }
    }
   
    // Small delay to prevent CPU hogging
    delay(10);
}

