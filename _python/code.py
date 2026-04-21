import time
import board
import busio
import digitalio
import adafruit_connection_manager
import adafruit_requests
import adafruit_minimqtt.adafruit_minimqtt as MQTT
import adafruit_rfm9x
from adafruit_esp32spi import adafruit_esp32spi

# --- CONFIGURATION ---
ssid = "gogogadgetnodes"
password = "st@rw@rs"
MQTT_BROKER = "192.168.68.108"
MQTT_PORT = 6683  # Updated custom port
MQTT_TOPIC = "lora/gateway/data"
RADIO_FREQ_MHZ = 915.0  # Match this to your transmitter

# --- HARDWARE SETUP ---
# SPI Bus (Shared by AirLift and RFM95)
spi = busio.SPI(board.SCK, board.MOSI, board.MISO)

# AirLift (WiFi) Pins
esp32_cs = digitalio.DigitalInOut(board.D13)
esp32_ready = digitalio.DigitalInOut(board.D11)
esp32_reset = digitalio.DigitalInOut(board.D12)

# RFM95 (LoRa) Pins 
# Based on Feather RP2040 RFM95 pinout
rfm95_cs = digitalio.DigitalInOut(board.RFM_CS)
rfm95_reset = digitalio.DigitalInOut(board.RFM_RST)

# Initialize WiFi
esp = adafruit_esp32spi.ESP_SPIcontrol(spi, esp32_cs, esp32_ready, esp32_reset)
pool = adafruit_connection_manager.get_radio_socketpool(esp)
ssl_context = adafruit_connection_manager.get_radio_ssl_context(esp)

# Initialize LoRa Radio
print("Initializing RFM95...")
rfm95 = adafruit_rfm9x.RFM9x(spi, rfm95_cs, rfm95_reset, RADIO_FREQ_MHZ)
rfm95.tx_power = 23 # Max power


# --- MQTT SETUP ---
mqtt_client = MQTT.MQTT(
    broker=MQTT_BROKER,
    port=MQTT_PORT,
    socket_pool=pool,
    ssl_context=None,
    client_id="feather_lora_gw" 
)

# --- WIFI & MQTT FUNCTIONS ---
def connect_wifi():
    if esp.is_connected:
        return
    print("Connecting to WiFi...")
    while not esp.is_connected:
        try:
            esp.connect_AP(ssid, password)
        except Exception as e:
            print(f"WiFi Error: {e}")
            time.sleep(2)
    
    # Robust IP printing fix
    raw_ip = esp.ipv4_address
    ip_str = ".".join(str(x) for x in raw_ip) if isinstance(raw_ip, (list, tuple, bytearray)) else str(raw_ip)
    print(f"Connected! IP: {ip_str}")


def connect_mqtt():
    if not mqtt_client.is_connected:
        print(f"Attempting MQTT connection to {MQTT_BROKER}:{MQTT_PORT}...")
        try:
            mqtt_client.connect()
            print("MQTT Connected!")
        except Exception as e:
            print(f"MQTT Connection Failed: {e}")
            return False
    return True

# Initialize MQTT Client
mqtt_client = MQTT.MQTT(
    broker=MQTT_BROKER,
    socket_pool=pool,
    ssl_context=ssl_context,
)

# --- MAIN RUNTIME ---
connect_wifi()
connect_mqtt()

last_received_time = time.monotonic()
print("Gateway active: Listening for LoRa packets...")

while True:
    # 1. Maintain WiFi and MQTT
    connect_wifi()
    is_mqtt_ok = connect_mqtt()
    # print (is_mqtt_ok)
    # 2. Check for LoRa Packets
    packet = rfm95.receive(timeout=0.5)
    current_time = time.monotonic()

    if packet is not None:
        last_received_time = current_time # Reset timer immediately
        try:
            packet_text = str(packet, "utf-8")
            print(f"LoRa Rx: {packet_text}")
            
            if is_mqtt_ok:
                mqtt_client.publish(MQTT_TOPIC, packet_text)
        except Exception as e:
            print(f"Relay Error: {e}")
    
    else:
        # 3. Handle 5-second Timeout
        if current_time - last_received_time >= 5.0:
            print("No LoRa data for 5s, sending heartbeat...")
            if is_mqtt_ok:
                try:
                    mqtt_client.publish(MQTT_TOPIC, 2)
                except Exception as e:
                    print(f"Publish failed: {e}")
            else:
                print("Skipping heartbeat: MQTT not connected.")
            
            # Reset timer to avoid spamming the "not connected" error
            last_received_time = current_time

    # Small sleep to yield to background tasks
    time.sleep(0.01)