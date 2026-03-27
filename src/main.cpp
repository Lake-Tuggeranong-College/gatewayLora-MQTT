#include <Arduino.h>
#include <SPI.h>

// To fix the 'IPAddress' error on RP2040, we must include the 
// Plugin-specific utility headers before the main WiFiNINA header.
#include <utility/wifi_drv.h>
#include <WiFiNINA.h>  
#include <PubSubClient.h>

/* --- CONFIGURATION --- */
#define RF95_FREQ 915.0         // Change to 433.0 if using that version
const char* ssid = "YOUR_WIFI_NAME";
const char* pass = "YOUR_WIFI_PASSWORD";
const char* broker_ip = "192.168.1.50"; // Your MQTT Broker IP
const char* mqtt_topic = "lora/gateway/data";

/* --- PIN DEFINITIONS --- */
#define RFM95_CS    16
#define RFM95_INT   21
#define RFM95_RST   17

// AirLift Pins for WiFiNINA
#define SPIWIFI_SS  13 
#define SPIWIFI_ACK 12
#define ESP32_RESET 11

/* --- GLOBAL OBJECTS --- */
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// For WiFiNINA v2+, defining our own instance avoids the 'setPins' 
// member error often found when the compiler uses the wrong WiFi class.
WiFiClass wifi_mod(SPIWIFI_SS, SPIWIFI_ACK, ESP32_RESET);
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Shared variables for core communication
char packetBuffer[RH_RF95_MAX_MESSAGE_LEN];
volatile bool hasNewPacket = false;

/* --- HELPER FUNCTIONS --- */

void connectToWiFi() {
  Serial.print("Connecting to WiFi...");
  // Use our wifi_mod instance
  while (wifi_mod.begin(ssid, pass) != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println(" Connected!");
}

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");
    if (mqttClient.connect("LoRaGateway_RP2040")) {
      Serial.println(" Connected!");
    } else {
      Serial.print(" failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// ==========================================
// CORE 1: RADIO LISTENER (The Radio Side)
// ==========================================

void setup1() {
  // Manual reset of LoRa radio (Required for RFM modules)
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH); delay(10);
  digitalWrite(RFM95_RST, LOW);  delay(10);
  digitalWrite(RFM95_RST, HIGH); delay(10);

  if (!rf95.init()) {
    Serial.println("Core 1: LoRa init failed!");
    while (1);
  }
  
  rf95.setFrequency(RF95_FREQ);
  rf95.setTxPower(23, false);
  Serial.println("Core 1: LoRa Listening...");
}

void loop1() {
  if (rf95.available()) {
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len)) {
      buf[len] = 0; // Null-terminate string
      
      // Copy to shared buffer ONLY if Core 0 has finished the previous packet
      if (!hasNewPacket) { 
        strcpy(packetBuffer, (char*)buf);
        hasNewPacket = true;
      }
    }
  }
}

// ==========================================
// CORE 0: WIFI & MQTT (The Internet Side)
// ==========================================

void setup() {
  Serial.begin(115200);
  delay(2000); // Wait for Serial to wake up

  // Pins are already configured via the wifi_mod constructor.
  connectToWiFi();
  mqttClient.setServer(broker_ip, 1883);
  Serial.println("Core 0: Network Ready.");
}

void loop() {
  // 1. Maintain Network Connection
  if (wifi_mod.status() != WL_CONNECTED) {
    connectToWiFi();
  }
  
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  
  mqttClient.loop();

  // 2. Check for data from Core 1
  if (hasNewPacket) {
    Serial.print("Gateway -> Publishing: ");
    Serial.println(packetBuffer);
    
    if (mqttClient.publish(mqtt_topic, packetBuffer)) {
      hasNewPacket = false; // Successfully sent, clear flag
    } else {
      Serial.println("MQTT Publish Failed!");
    }
  }
}


