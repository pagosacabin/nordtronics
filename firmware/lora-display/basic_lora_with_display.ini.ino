/**
 * Send and receive LoRa-modulation packets with a sequence number, showing RSSI
 * and SNR for received packets on the little display.
 *
 * Note that while this send and received using LoRa modulation, it does not do
 * LoRaWAN. For that, see the LoRaWAN_TTN example.
 *
 * This works on the stick, but the output on the screen gets cut off.
*/
// Turns the 'PRG' button into the power button, long press is off 
#define HELTEC_POWER_BUTTON   // must be before "#include <heltec_unofficial.h>"
#include <heltec_unofficial.h>
#include <UltrasonicA02YYUW.h>

#include <PubSubClient.h>

//Captive portal
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ESPmDNS.h>

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;
WebServer webServer(80);
Preferences preferences;


// MQTT Broker settings

// Network credentials
//const char* ssid = "NordNickell";
//const char* password = "bethany83";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
const char* mqtt_server = "192.168.1.132"; // e.g., "192.168.1.100"
const int mqtt_port = 1883;

UltrasonicA02YYUW sensor(Serial1, 5, 6); // RX, TX

// Pause between transmited packets in seconds.
// Set to zero to only transmit a packet when pressing the user button
// Will not exceed 1% duty cycle, even if you set a lower value.
#define PAUSE               30

// Frequency in MHz. Keep the decimal point to designate float.
// Check your own rules and regulations to see what is legal where you are.
//#define FREQUENCY           866.3       // for Europe
#define FREQUENCY           910.525      // for US

// LoRa bandwidth. Keep the decimal point to designate float.
// Allowed values are 7.8, 10.4, 15.6, 20.8, 31.25, 41.7, 62.5, 125.0, 250.0 and 500.0 kHz.
#define BANDWIDTH           250.0

// Number from 5 to 12. Higher means slower but higher "processor gain",
// meaning (in nutshell) longer range and more robust against interference. 
#define SPREADING_FACTOR    9

// Transmit power in dBm. 0 dBm = 1 mW, enough for tabletop-testing. This value can be
// set anywhere between -9 dBm (0.125 mW) to 22 dBm (158 mW). Note that the maximum ERP
// (which is what your antenna maximally radiates) on the EU ISM band is 25 mW, and that
// transmissting without an antenna can damage your hardware.
#define TRANSMIT_POWER      1


String rxdata;
volatile bool rxFlag = false;
long counter = 0;
uint64_t last_tx = 0;
uint64_t tx_time;
uint64_t minimum_pause;

/*void setup_wifi() {
  delay(10);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected. IP: ");
  Serial.println(WiFi.localIP());
}
*/

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  heltec_setup();
  Serial.begin(9600);
//setup_wifi();
//
  preferences.begin("device-config", false);

  // 1. Load all stored data
  String ssid = preferences.getString("ssid", "");
  String pass = preferences.getString("pass", "");
  String host = preferences.getString("host", "ESP32-Device");
  float distEmpty = preferences.getFloat("distEmpty", 0.0);
  float distFull = preferences.getFloat("distFull", 0.0);

  if (ssid != "") {
    // Set Hostname BEFORE WiFi starts to ensure DHCP registration
    WiFi.disconnect(true);
    WiFi.setHostname(host.c_str());
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

    Serial.print("Connecting to [" + ssid + "] as [" + host + "]...");
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
      delay(500); Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nCONNECTED! IP Address: " + WiFi.localIP().toString());
      Serial.println("Thresholds: Empty=" + String(distEmpty) + "cm, Full=" + String(distFull) + "cm");
      MDNS.begin(host.c_str());
      return; 
    }
    Serial.println("\nConnection failed. Launching portal...");
  }

  // 2. Start Secure Setup Portal & Network Scan
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("ESP32-Setup-Portal", "setup1234"); // Secure AP Password
  
  dnsServer.start(DNS_PORT, "*", apIP);
  Serial.println("Portal active at: " + WiFi.softAPIP().toString());

  webServer.onNotFound([]() {
    int n = WiFi.scanNetworks();
    String options = "";
    if (n <= 0) {
      options = "<option disabled selected>No networks found</option>";
    } else {
      for (int i = 0; i < n; ++i) {
        options += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + " (" + String(WiFi.RSSI(i)) + " dBm)</option>";
      }
    }

    String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'>"
      "<style>body{font-family:sans-serif; display:flex; justify-content:center; align-items:center; height:100vh; margin:0; background:#f0f2f5;}"
      ".card{background:white; padding:35px; border-radius:20px; box-shadow:0 12px 30px rgba(0,0,0,0.15); width:95%; max-width:400px; text-align:center;}"
      "h2{color:#1a1a1a; margin-bottom:25px;} label{display:block; text-align:left; font-size:13px; color:#555; margin-top:10px; font-weight:600;}"
      "select, input{width:100%; padding:14px; margin:8px 0; border:1px solid #ccd0d5; border-radius:10px; box-sizing:border-box; font-size:16px;}"
      ".toggle-box{text-align:left; font-size:14px; color:#666; margin:5px 0 15px 0; display:flex; align-items:center;}"
      "input[type='submit']{background:#007bff; color:white; border:none; cursor:pointer; font-weight:bold; margin-top:20px; transition:0.2s;}"
      "input[type='submit']:hover{background:#0056b3;}</style>"
      "<script>function togglePass(){var x=document.getElementById('p'); x.type=(x.type==='password')?'text':'password';}</script></head><body>"
      "<div class='card'><h2>Device Setup</h2><form action='/save' method='POST'>"
      "<label>WiFi Network</label><select name='s' required>" + options + "</select>"
      "<label>WiFi Password</label><input type='password' name='p' id='p' placeholder='Enter password' required>"
      "<div class='toggle-box'><input type='checkbox' onclick='togglePass()' style='width:auto; margin-right:8px;'>Show Password</div>"
      "<label>Device Hostname</label><input type='text' name='h' value='ESP32-Device' required>"
      "<label>Empty Distance (cm)</label><input type='number' step='0.1' name='de' placeholder='Distance when empty' required>"
      "<label>Full Distance (cm)</label><input type='number' step='0.1' name='df' placeholder='Distance when full' required>"
      "<input type='submit' value='Save & Restart Device'></form></div></body></html>";
    webServer.send(200, "text/html", html);
  });

  webServer.on("/save", []() {
    // 3. Save all parameters to NVS
    preferences.putString("ssid", webServer.arg("s"));
    preferences.putString("pass", webServer.arg("p"));
    preferences.putString("host", webServer.arg("h"));
    preferences.putFloat("distEmpty", webServer.arg("de").toFloat());
    preferences.putFloat("distFull", webServer.arg("df").toFloat());
    
    webServer.send(200, "text/html", "Settings Saved. Device is rebooting...");
    delay(2000);
    ESP.restart();
  });

  webServer.begin();
  //

  Serial.println(WiFi.localIP());
  client.setServer(mqtt_server, mqtt_port);

  sensor.begin();
  VextON();
  display.setFont(ArialMT_Plain_10);
  //display.println("Radio init");
  
  RADIOLIB_OR_HALT(radio.begin());
  // Set the callback function for received packets
  radio.setDio1Action(rx);
  // Set radio parameters
  //both.printf("Frequency: %.2f MHz\n", FREQUENCY);
  RADIOLIB_OR_HALT(radio.setFrequency(FREQUENCY));
  //both.printf("Bandwidth: %.1f kHz\n", BANDWIDTH);
  RADIOLIB_OR_HALT(radio.setBandwidth(BANDWIDTH));
  //both.printf("Spreading Factor: %i\n", SPREADING_FACTOR);
  RADIOLIB_OR_HALT(radio.setSpreadingFactor(SPREADING_FACTOR));
  //both.printf("TX power: %i dBm\n", TRANSMIT_POWER);
  RADIOLIB_OR_HALT(radio.setOutputPower(TRANSMIT_POWER));
  // Start receiving
  RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF));
  //display.clear();
}

void VextON(void)
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, LOW);
}

void VextOFF(void) //Vext default OFF
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, HIGH);
}

// Can't do Serial or display things here, takes too much time for the interrupt
void rx() 
{
  rxFlag = true;
}

String device = "Transmitter";
String txString = "No Data has been sent";
String rxString = "No Data received";
String strPercentFull = "Unknown";
float percentFull = 0.00;
float rxDistance = 0.00;
float maxFull = 30;
float minFull = 140;

void loop() {
  heltec_loop();


  // mqtt stuff
/*
    if (!client.connected()) {
    reconnect();
  }
  client.loop(); // Essential to maintain connection
  */
  webServer.handleClient();
  unsigned long now = millis();
  if (now - lastMsg > 5000) { // Send every 5 seconds
    lastMsg = now;
    String payload = "Hello from ESP32";
    client.publish("esp32/test", payload.c_str());
    Serial.println("Published: " + payload);
  }
  
  // Update and get sensor data
  sensor.update();  // panggil rutin di loop
  float d = sensor.getDistance();
  float my_float;


  //Serial.println(String(percentFull));
  //Serial.println(String(d));

  //Identify device RX/TX on device with sensor with transmitt
  if (d == -1){
    device = "Receiver";
  }

  //Breakout for Receiver vs. Transmitter logic
  if ( device == "Receiver"){
    //Receiver code
    display.drawString(64/4, 0,device);
    display.drawHorizontalLine(0, 13, 128);
    display.drawString(64/4,15,"distance: " + rxString);
    display.drawString(64/4,25,"%Full: " + strPercentFull);
    if (percentFull > 100){
      percentFull = 100;
    }
    display.drawProgressBar(15,40,100,15,percentFull);

    float rxDistance = rxString.toFloat();
    percentFull = 100*((minFull+(maxFull-rxDistance))/minFull);
    strPercentFull = String(percentFull);
    

    // If a packet was received, display it and the RSSI and SNR
    if (rxFlag) {
      rxFlag = false;
      radio.readData(rxdata);
      if (_radiolib_status == RADIOLIB_ERR_NONE) {
        Serial.printf("RX [%s]\n", rxdata.c_str());
        rxString = String(rxdata.c_str());
        //my_float = std::stof(rxString);
        //Display output to Serial Monitor
        Serial.printf("  RSSI: %.2f dBm\n", radio.getRSSI());
        Serial.printf("  SNR: %.2f dB\n", radio.getSNR());
      }
      RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF));
    }
  } else {
    //Transmitter code
    display.drawString(64/4,0,device);
    display.drawHorizontalLine(0, 13, 128);
    display.drawString(64/4, 15, "Dis: " + String(d) + " cm");
    display.drawString(64/4,25,txString);

    display.drawString(64/4,35,"%Full: " + strPercentFull);
    if (percentFull > 100){
      percentFull = 100;
    }
    display.drawProgressBar(15,48,100,15,percentFull);

    float txDistance = String(d).toFloat();
    percentFull = 100*((minFull+(maxFull-txDistance))/minFull);
    strPercentFull = String(percentFull);


    bool tx_legal = millis() > last_tx + minimum_pause;
    // Transmit a packet every PAUSE seconds or when the button is pressed
    if ((PAUSE && tx_legal && millis() - last_tx > (PAUSE * 1000)) || button.isSingleClick()) {
    
      // In case of button click, tell user to wait
      if (!tx_legal) {
        //display.printf("Legal limit, wait %i sec.\n", (int)((minimum_pause - (millis() - last_tx)) / 1000) + 1);
        return;
      }
    
      //displayed on Local Display
      txString = "TX [Distance: " + String(d) + "]";

      // LoRa Stuff
      radio.clearDio1Action();
      heltec_led(50); // 50% brightness is plenty for this LED
      tx_time = millis();

      //Text to Displayed on Remote display
      String strcounter = String(d);

      //Transmitting LoRa Packett
      RADIOLIB(radio.transmit(strcounter.c_str()));
      tx_time = millis() - tx_time;
      heltec_led(0);
      if (_radiolib_status == RADIOLIB_ERR_NONE) {
        Serial.printf("OK (%i ms)\n", (int)tx_time);
      } else {
        Serial.printf("fail (%i)\n", _radiolib_status);
      }
      // Maximum 1% duty cycle
      minimum_pause = tx_time * 100;
      last_tx = millis();
      radio.setDio1Action(rx);
      RADIOLIB_OR_HALT(radio.startReceive(RADIOLIB_SX126X_RX_TIMEOUT_INF));
    }
  }
  //Draw display
  display.display();

  //Clear display
  display.clear();
  delay(500);
  
}


