/*
  Controle do rádio RDA5807 via MQTT
  Exemplos de comandos usando mosquitto:

  # Mudar frequência para 103.9 MHz (10390 kHz)
  mosquitto_pub -h <BROKER_IP> -t /home/RDA4807/frequency -m "10390"

  # Mudar volume para 10
  mosquitto_pub -h <BROKER_IP> -t /home/RDA4807/volume -m "10"

  # Você pode usar o mosquitto_sub para monitorar os tópicos
  mosquitto_sub -h <BROKER_IP> -t /home/RDA4807/frequency
  mosquitto_sub -h <BROKER_IP> -t /home/RDA4807/volume

  Placa recomendada: ESP32 ou ESP8266
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <RDA5807.h>

const char* ssid = "SEU_WIFI";
const char* password = "SUA_SENHA";
const char* mqtt_server = "BROKER_IP";

WiFiClient espClient;
PubSubClient client(espClient);
RDA5807 rx;

void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];

  if (strcmp(topic, "/home/RDA4807/frequency") == 0) {
    int freq = msg.toInt();
    rx.setFrequency(freq); // freq em kHz, ex: 10390 para 103.9 MHz
  }
  if (strcmp(topic, "/home/RDA4807/volume") == 0) {
    int vol = msg.toInt();
    rx.setVolume(vol); // volume de 0 a 15
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
    if (client.connect("RDA5807Client")) {
      client.subscribe("/home/RDA4807/frequency");
      client.subscribe("/home/RDA4807/volume");
    } else {
      delay(5000);
    }
  }
}

void setup() {
  rx.setup();
  rx.setFrequency(10390);
  rx.setVolume(8);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();
}
