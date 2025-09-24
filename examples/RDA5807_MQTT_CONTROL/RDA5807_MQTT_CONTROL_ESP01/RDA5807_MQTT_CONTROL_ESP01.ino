/*
  This sketch is an example of using the ESP01 (IoT Module based on ESP8266) to control an FM receiver 
  based on the RDA5807 DSP using the MQTT protocol. To control the RDA5807, this sketch uses the library 
  developed by me and available on the Arduino platform (available at https://github.com/pu2clr/RDA5807).

  RDA5807 radio control via MQTT

  For more information about the RDA5807, please check the GitHub repository: https://github.com/pu2clr/RDA5807
  

  ESP8266 Dev Module Wire up
  | Device name               | RDA5807 Pin          | ESP8266 Dev Module |
  | ------------------------- | -------------------- | ------------------ |
  | RDA5807                   |                      |      ESP01         | 
  |                           | VCC                  |      3.3V          |
  |                           | GND                  |      GND           |    
  |                           | SDIO / SDA (pin 2)   |      GPIO0         |
  |                           | SCLK (pin 1)         |      GPIO2         |
  | ------------------------- | -------------------- | ------------------ |

  #  Examples of commands using mosquitto:

  # Change frequency to 103.9 MHz (10390 kHz)
  mosquitto_pub -h <BROKER_IP> -t home/RDA5807/frequency -m "10390"

  # Change volume to 10
  mosquitto_pub -h <BROKER_IP> -t home/RDA5807/volume -m "10"

  # You can use mosquitto_sub to monitor the topics and JSON status
  mosquitto_sub -h <BROKER_IP> -t home/RDA5807/frequency
  mosquitto_sub -h <BROKER_IP> -t home/RDA5807/volume
  mosquitto_sub -h <BROKER_IP> -t home/RDA5807/status

  Tests:

  mosquitto_pub -h 192.168.1.102  -u homeguard  -P pu2clr123456  -t "home/RDA5807/volume" -m "10"
  mosquitto_pub -h 192.168.1.102  -u homeguard  -P pu2clr123456  -t "home/RDA5807/frequency" -m "10390"

  # Monitor status in JSON format:
  mosquitto_sub -h 192.168.1.102 -u homeguard -P pu2clr123456 -t "home/RDA5807/status" -v

  JSON Status Format:
  {
    "device_id": "RDA5807",
    "name": "Rádio DSP", 
    "location": "Quarto",
    "ip": "192.168.1.xxx",
    "frequency": 10390,
    "volume": 9,
    "command": "frequency|volume|init|reconnect",
    "value": "command_value",
    "action": "human_readable_description",
    "timestamp": "milliseconds_since_boot"
  }

  Recommended board: ESP32 or ESP8266

  Author: Ricardo Lima Caratti.

*/


#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <RDA5807.h>

#define ESP01_I2C_SDA 0     // GPIO0
#define ESP01_I2C_SCL 2     // GPIO2 

const char* ssid = "Homeguard";
const char* password = "pu2clr123456";
const char* mqtt_server = "192.168.1.102";
const char* mqtt_user = "homeguard";
const char* mqtt_pass = "pu2clr123456";

// ======== Device Information ========
#define DEVICE_ID        "RDA5807"
#define DEVICE_NAME      "Rádio DSP"
#define DEVICE_LOCATION  "Quarto"

WiFiClient espClient;
PubSubClient client(espClient);
RDA5807 rx;

// ======== Current Device State ========
int currentFrequency = 10390;  // Default frequency in kHz
int currentVolume = 9;         // Default volume (0-15)

// ======== Publish Device Status JSON ========
void publishDeviceStatus(const String& command, const String& value, const String& action) {
  String payload = "{";
  payload += "\"device_id\":\"" + String(DEVICE_ID) + "\",";
  payload += "\"name\":\"" + String(DEVICE_NAME) + "\",";
  payload += "\"location\":\"" + String(DEVICE_LOCATION) + "\",";
  payload += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
  payload += "\"frequency\":" + String(currentFrequency) + ",";
  payload += "\"volume\":" + String(currentVolume) + ",";
  payload += "\"command\":\"" + command + "\",";
  payload += "\"value\":\"" + value + "\",";
  payload += "\"action\":\"" + action + "\",";
  payload += "\"timestamp\":\"" + String(millis()) + "\"";
  payload += "}";
  
  client.publish("home/RDA5807/status", payload.c_str(), false);
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];

  if (strcmp(topic, "home/RDA5807/frequency") == 0) {
    int freq = msg.toInt();
    currentFrequency = freq;  // Update current frequency
    rx.setFrequency(freq); // freq em kHz, ex: 10390 para 103.9 MHz
    
    // Publish status with device information
    String freqMHz = String(freq / 100.0, 1) + " MHz";
    publishDeviceStatus("frequency", msg, "Frequência alterada para " + freqMHz);
  }
  if (strcmp(topic, "home/RDA5807/volume") == 0) {
    int vol = msg.toInt();
    currentVolume = vol;  // Update current volume
    rx.setVolume(vol); // volume de 0 a 15
    
    String action;
    if (vol == 0) {
      rx.setMute(true);
      action = "Volume zerado - Rádio silenciado";
    } else {
      rx.setMute(false);
      action = "Volume alterado para " + msg + "/15";
    }
    
    // Publish status with device information
    publishDeviceStatus("volume", msg, action);
  }
}

void setup_wifi() {

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
 
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("RDA5807Client", mqtt_user, mqtt_pass)) {
      client.subscribe("home/RDA5807/frequency");
      client.subscribe("home/RDA5807/volume");
      
      // Publish reconnection status
      publishDeviceStatus("reconnect", "success", "Reconectado ao broker MQTT");
      
    } else {
      delay(5000);
    }
  }
}

void setup() {

  Wire.begin(ESP01_I2C_SDA, ESP01_I2C_SCL);
  rx.setup();
  delay(300);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // Connect to MQTT and wait for connection
  while (!client.connected()) {
    if (client.connect("RDA5807Client", mqtt_user, mqtt_pass)) {
      client.subscribe("home/RDA5807/frequency");
      client.subscribe("home/RDA5807/volume");
      
      // Publish initial device information
      publishDeviceStatus("init", "startup", "Dispositivo RDA5807 inicializado com sucesso");
      
      break;
    } else {
      delay(5000);
    }
  }

  // Set initial frequency and volume
  rx.setFrequency(10390);
  delay(100);
  rx.setVolume(9);
  
  // Publish initial settings
  publishDeviceStatus("frequency", "10390", "Frequência inicial configurada para 103.9 MHz");
  publishDeviceStatus("volume", "9", "Volume inicial configurado para 9/15");
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();
}
