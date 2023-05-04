#include "Mqtt.h"

int Mqtt::statePump = 1;

double Mqtt::calibrationFactor = 4.5;

int &Mqtt::getStatePump(){
  return statePump;
}

double &Mqtt::getCalibrationFactor(){
  return calibrationFactor;
}

// Default configuraion
Mqtt::Mqtt(WiFiClientSecure &espClient):PubSubClient(espClient){
  // If is a public IP address
  setServer(mqttServer, port);
  espClient.setCACert(mqttCertificates[HIVEMQ_CERTIFICATE]);
  setCallback([this](char* topic, uint8_t* payload, unsigned int length) {
            callbackFunc(topic, payload, length, *this);
        });

  // If is a private IP address
  // setServer(mqttServer, port);
  // setCallback(callbackFunc);
}

Mqtt::Mqtt(WiFiClient &espClient, char *server, int port):PubSubClient(espClient){
	setServer(mqttServer, port);
  setCallback([this](char* topic, uint8_t* payload, unsigned int length) {
            callbackFunc(topic, payload, length, *this);
        });
}

Mqtt::Mqtt(WiFiClientSecure &espClient, char *server, int port, int chosenCertificate):PubSubClient(espClient){
	setServer(server, port);
  espClient.setCACert(mqttCertificates[chosenCertificate]);
  setCallback([this](char* topic, uint8_t* payload, unsigned int length) {
            callbackFunc(topic, payload, length, *this);
        });

}
Mqtt::Mqtt(WiFiClientSecure &espClient, char *server, int port, char *certificate):PubSubClient(espClient){
	setServer(server, port);
  espClient.setCACert(certificate);
  setCallback([this](char* topic, uint8_t* payload, unsigned int length) {
            callbackFunc(topic, payload, length, *this);
        });
}


void Mqtt::reconnect(){
  while (!this->connected()) {
    Serial.print("Attempting MQTT connection...");
    rand = random(1000);
    sprintf(clientId, "clientId-%ld", rand);
    if (this->connect(clientId,mqttUser,mqttPassword)) {
      Serial.print(clientId);
      Serial.println(" connected");
      subscribe("actuator/water_pump");
      subscribe("sensor/water_flow/factor");
      subscribe("debug/sensor/water_flow/factor");
    } else {
      Serial.print("failed, rc=");
      Serial.print(this->state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
      continue;
    }
  }
 }

 void Mqtt::callbackFunc(char* topic, byte* message, unsigned int length, PubSubClient &client) {
      String stMessage;
      
      for (int i = 0; i < length; i++) {
        stMessage += (char)message[i];
      }
      if (String(topic) == "actuator/water_pump") {
        Serial.println("water_pump: " + stMessage);
          if(stMessage == "1" || stMessage == "0"){
            statePump = stMessage.toInt();
          }
          else {
            client.publish("debug/actuator/water_pump","The number must to be 1 or 0");
          }
        }
        if(String(topic) == "sensor/water_flow/factor"){
        Serial.println("factor: " + stMessage);
        if(stMessage.toDouble() > 0){
          calibrationFactor = stMessage.toDouble();
        }
        else{
          client.publish("debug/sensor/water_flow/factor","The number must be greater than 0");
        }

        }
        
      }

void Mqtt::wifiConnect() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.macAddress());
}
    
