#include "Mqtt.h"

// Default configuraion
Mqtt::Mqtt(WiFiClientSecure &espClient):PubSubClient(espClient){
  // If is a public IP address
  setServer(mqttServer, port);
  espClient.setCACert(mqttCertificates[HIVEMQ_CERTIFICATE]);
  setCallback(callbackFunc);

  // If is a private IP address
  // setServer(mqttServer, port);
  // setCallback(callbackFunc);
}

Mqtt::Mqtt(WiFiClient &espClient, char *server, int port):PubSubClient(espClient){
	setServer(server, port);
  setCallback(callbackFunc);
}

Mqtt::Mqtt(WiFiClientSecure &espClient, char *server, int port, int chosenCertificate):PubSubClient(espClient){
	setServer(server, port);
  espClient.setCACert(mqttCertificates[chosenCertificate]);
  setCallback(callbackFunc);

}
Mqtt::Mqtt(WiFiClientSecure &espClient, char *server, int port, char *certificate):PubSubClient(espClient){
	setServer(server, port);
  espClient.setCACert(certificate);
  setCallback(callbackFunc);
}

void Mqtt::reconnect(){
  while (!this->connected()) {
    Serial.print("Attempting MQTT connection...");
    long r = random(1000);
    sprintf(clientId, "clientId-%ld", r);
    if (this->connect(clientId,mqttUser,mqttPassword)) {
      Serial.print(clientId);
      Serial.println(" connected");
      subscribe("topicName/led");
      subscribe("reset");
    } else {
      Serial.print("failed, rc=");
      Serial.print(this->state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
      continue;
    }
  }
 }

 void Mqtt::callbackFunc(char* topic, byte* message, unsigned int length) {
      Serial.print("Message arrived on topic: ");
      Serial.println(topic);
      String stMessage;
      
      for (int i = 0; i < length; i++) {
        stMessage += (char)message[i];
      }

      if (String(topic) == "sensors/water_flow") {
        Serial.println("Flowing " + stMessage + " L");
      }
      if (String(topic) == "sensors/soil_moisture") {
        Serial.println("Soil moisture: " + stMessage);
      }
      if (String(topic) == "sensors/lux") {
        Serial.println("Lux: " + stMessage);
      }
      if (String(topic) == "oled/emotion") {
        Serial.println("Oled emotion: " + stMessage);
      }
    }
