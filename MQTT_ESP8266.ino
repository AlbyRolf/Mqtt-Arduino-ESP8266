#include <ESP8266WiFi.h>
#include <PubSubClient.h>
WiFiClient espClient;
PubSubClient client(espClient);
long lastReconnectAttempt = 0;
//WIFI CONFIGURATION
const char *ssid = "SSID-WIFI";
const char *password = "PasswordWifi";
// MQTT Broker
const char *id_client = "CLIENT1";//I USE MOSQUITTO
const char *mqtt_broker = "BROKER_IP";//I USE MOSQUITTO
const char *mqtt_username = "username";
const char *mqtt_password = "password";
const int mqtt_port = 1883; //default MQTT Port
const char *WillMessage = "Disconnect";
//MQTT TOPIC
const char *Client2Status = "Esp/Client2Status"; //For recive Connection or Disconnection from specific client
const char *Client1Status = "Esp/Client1Status"; //For recive Connection or Disconnection from specific client
//Start Configuration Wifi
void initWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to Wifi");
  while (WiFi.status() != WL_CONNECTED) { //loop Connection
    delay(1000);
    Serial.println("_");
  }
  Serial.println("Wifi Connected");
  //IMPORTANT !!!
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
}
//Start Configuration Mqtt
void initMqtt() {//inizio colegamento MQtt
  client.setCallback(callback); //callback when broker publish message
  client.setServer(mqtt_broker, mqtt_port);
  Serial.println("Try Connect to MQTT Broker");
  while (!client.connected()) {
    if (client.connect(id_client, mqtt_username, mqtt_password, Client1Status, 2, false, WillMessage)) {
      client.publish(Client1Status, "Connected", false);
      client.subscribe(Client2Status);
      Serial.println("MQTT Broker Connected");
    } else {
      Serial.println("MQTT Broker Connection fail...Reconnect");
    }
  }
}
//When Broker Mosquitto Publish Message
void callback(char *topic, byte *payload, unsigned int length) {// callback msg recived broker
  if (strcmp(topic, Client2Status) == 0) {//quando mi arriva un messaggio di stato dal Broker
    if (!strncmp((char *)payload, "Connected", length)) { //Broker connesso
      Serial.println("CLient2 Connected");
    } else {//Broker Disconnesso
      Serial.println("CLient2 Disconnected");
    }
  }

}

boolean reconnect() {//Reconnect to Mqtt Broker
  if (client.connect(id_client, mqtt_username, mqtt_password, Client1Status, 2, false, WillMessage)) {
    client.publish(Client1Status, "Connected", false);
    client.subscribe(Client2Status);
    Serial.println("MQTT Broker Connected");
  }
  return client.connected();
}
void setup() {
  Serial.begin(115200);
  delay(1000);
  initWifi();
  delay(1000);
  initMqtt();
}
void loop() {
  if (!client.connected()) { //Check MQTT Connection
    Serial.println("MQTT Broker Connection fail...Reconnect");
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      //Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    client.loop();
  }
  if (WiFi.status() != WL_CONNECTED) { //Check Wifi Connection
   Serial.println("Wifi Disonnected");
  }
}
