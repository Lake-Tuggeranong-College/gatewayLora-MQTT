#include <Arduino.h>
#include <SPI.h>
#include <RH_RF95.h>  // Include RadioHead LoRa driver

// To fix the 'IPAddress' error on RP2040, we must include the 
// Plugin-specific utility headers before the main WiFiNINA header.
// #include <utility/wifi_drv.h>
#include <WiFiNINA.h>  

// AirLift Pins (configured for the RP2040 Feather)
#define SPIWIFI_SS    13
#define SPIWIFI_ACK    11
#define ESP32_RESETN   12

// #include <PubSubClient.h>

// /* --- CONFIGURATION --- */
// #define RF95_FREQ 915.0         // Change to 433.0 if using that version
const char* ssid = "gogogadgetnodes";
const char* pass = "st@rw@rs";
// const char* broker_ip = "192.168.68.108"; // Your MQTT Broker IP
// const char* mqtt_topic = "modules/newData";

// /* --- PIN DEFINITIONS --- */
// #define RFM95_CS    16
// #define RFM95_INT   21
// #define RFM95_RST   17

// // // AirLift Pins for WiFiNINA
  #define SPIWIFI       SPI
  #define SPIWIFI_SS    13   // Chip select pin
  #define SPIWIFI_ACK    11   // a.k.a BUSY or READY pin
  #define ESP32_RESETN   12   // Reset pin
  #define ESP32_GPIO0   -1   // Not connected

// /* --- GLOBAL OBJECTS --- */
// RH_RF95 rf95(RFM95_CS, RFM95_INT);

// // For WiFiNINA v2+, defining our own instance avoids the 'setPins' 
// // member error often found when the compiler uses the wrong WiFi class.
// WiFiClient wifiClient;
// int status = WL_IDLE_STATUS;     // the Wifi radio's status

// PubSubClient mqttClient(wifiClient);

// // Shared variables for core communication
// char packetBuffer[RH_RF95_MAX_MESSAGE_LEN];
// volatile bool hasNewPacket = false;

// /* --- HELPER FUNCTIONS --- */

// void printMacAddress(byte mac[]) {
//   for (int i = 5; i >= 0; i--) {
//     if (mac[i] < 16) {
//       Serial.print("0");
//     }
//     Serial.print(mac[i], HEX);
//     if (i > 0) {
//       Serial.print(":");
//     }
//   }
//   Serial.println();
// }


// void printWifiData() {
//   // print your board's IP address:
//   IPAddress ip = WiFi.localIP();
//   Serial.print("IP Address: ");
//   Serial.println(ip);
//   Serial.println(ip);

//   // print your MAC address:
//   byte mac[6];
//   WiFi.macAddress(mac);
//   Serial.print("MAC address: ");
//   printMacAddress(mac);
// }

// void connectToWiFi() {
//   while (status != WL_CONNECTED) {
//     Serial.print("Attempting to connect to WPA SSID: ");
//     Serial.println(ssid);
//     // Connect to WPA/WPA2 network:
//     status = WiFi.begin(ssid, pass);

//     // wait 10 seconds for connection:
//     delay(10000);
//   }
//   Serial.print("You're connected to the network");
//   printWifiData();
// }



// void reconnectMQTT() {
//   while (!mqttClient.connected()) {
//     Serial.print("Connecting to MQTT...");
//     if (mqttClient.connect("LoRaGateway_RP2040")) {
//       Serial.println(" Connected!");
//     } else {
//       Serial.print(" failed, rc=");
//       Serial.print(mqttClient.state());
//       Serial.println(" try again in 5 seconds");
//       delay(5000);
//     }
//   }
// }

// // ==========================================
// // CORE 1: RADIO LISTENER (The Radio Side)
// // ==========================================

// void setup1() {
//   // Manual reset of LoRa radio (Required for RFM modules)
//   pinMode(RFM95_RST, OUTPUT);
//   digitalWrite(RFM95_RST, HIGH); delay(10);
//   digitalWrite(RFM95_RST, LOW);  delay(10);
//   digitalWrite(RFM95_RST, HIGH); delay(10);

//   if (!rf95.init()) {
//     Serial.println("Core 1: LoRa init failed!");
//     while (1);
//   }
  
//   rf95.setFrequency(RF95_FREQ);
//   rf95.setTxPower(23, false);
//   Serial.println("Core 1: LoRa Listening...");
// }

// void loop1() {
//   if (rf95.available()) {
//     uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
//     uint8_t len = sizeof(buf);

//     if (rf95.recv(buf, &len)) {
//       buf[len] = 0; // Null-terminate string
      
//       // Copy to shared buffer ONLY if Core 0 has finished the previous packet
//       if (!hasNewPacket) { 
//         strcpy(packetBuffer, (char*)buf);
//         hasNewPacket = true;
//       }
//     }
//   }
// }

// // ==========================================
// // CORE 0: WIFI & MQTT (The Internet Side)
// // ==========================================

// void setup() {
//   Serial.begin(115200);
//   delay(2000); // Wait for Serial to wake up

//   // Pins are already configured via the wifi_mod constructor.
//   connectToWiFi();
//   mqttClient.setServer(broker_ip, 6683);
//   Serial.println("Core 0: Network Ready.");
//   WiFi.setPins(SPIWIFI_SS, SPIWIFI_ACK, ESP32_RESETN, ESP32_GPIO0, &SPIWIFI);

// }

// void loop() {
//   // 1. Maintain Network Connection
//   // if (wifi_mod.status() != WL_CONNECTED) {
//   //   connectToWiFi();
//   // }
  
//   if (!mqttClient.connected()) {
//     reconnectMQTT();
//   }
  
//   mqttClient.loop();

//   // 2. Check for data from Core 1
//   if (hasNewPacket) {
//     Serial.print("Gateway -> Publishing: ");
//     Serial.println(packetBuffer);
    
//     if (mqttClient.publish(mqtt_topic, packetBuffer)) {
//       hasNewPacket = false; // Successfully sent, clear flag
//     } else {
//       Serial.println("MQTT Publish Failed!");
//     }
//   }
// }




/*

 This example connects to an unencrypted Wifi network.
 Then it prints the  MAC address of the Wifi module,
 the IP address obtained, and other network details.

 created 13 July 2010
 by dlf (Metodo2 srl)
 modified 31 May 2012
 by Tom Igoe
 */
#include <SPI.h>
#include <WiFiNINA.h>
// #include "pin_config.h" // Configure the pins used for the ESP32 connection
// #include "arduino_secrets.h" 

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
// char ssid[] = ssid;        // your network SSID (name)
// char pass[] = pass;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;     // the Wifi radio's status
#define RFM95_CS      16  // The Chip Select pin for the LoRa radio

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}


void printWifiData() {
  Serial.println("--- Entering printWifiData ---");
  
  Serial.print("Requesting IP... ");
  IPAddress ip = WiFi.localIP(); 
  // If the line above hangs, you will never see the next print.
  Serial.println("Done!"); 

  Serial.print("IP Address: ");
  Serial.println(ip);

  Serial.print("Requesting MAC... ");
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.println("Done!");

  Serial.print("MAC address: ");
  printMacAddress(mac);
  
  Serial.println("--- Exiting printWifiData ---");
}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  printMacAddress(bssid);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
  Serial.println();
  Serial.println("after current net");

}


void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
while (!Serial && millis() < 5000);

// --- ADDED: DISABLE LORA RADIO ---
  // We must pull the RFM95 CS pin HIGH to prevent SPI bus contention.
  pinMode(RFM95_CS, OUTPUT);
  digitalWrite(RFM95_CS, HIGH); 
  // ---------------------------------

SPI.begin();

  WiFi.setPins(SPIWIFI_SS, SPIWIFI_ACK, ESP32_RESETN, ESP32_GPIO0, &SPIWIFI);
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  // you're connected now, so print out the data:
  Serial.print("You're connected to the network");
  // printCurrentNet();
 Serial.println("before wifi data");
  printWifiData();
  Serial.println("after wifi data");

}

void loop() {
  // check the network connection once every 10 seconds:
  delay(10000);
  printCurrentNet();
}
