#include "Mqtt.h"


int Mqtt::statePump = 1;

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
	setServer(server, port);
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

int *Mqtt::getStatePump(){
  return &statePump;
}

void Mqtt::reconnect(){
  while (!this->connected()) {
    Serial.print("Attempting MQTT connection...");
    long r = random(1000);
    sprintf(clientId, "clientId-%ld", r);
    if (this->connect(clientId,mqttUser,mqttPassword)) {
      Serial.print(clientId);
      Serial.println(" connected");
      subscribe("actuators/water_pump");
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
      if (String(topic) == "actuators/water_pump") {
        Serial.println("water_pump: " + stMessage);
          if(stMessage == "1" || stMessage == "0"){
            statePump = stMessage.toInt();
          }
          else {
            client.publish("debug/actuators/water_pump","The state must to be 1 or 0");
          }
        }
        
      }
    
