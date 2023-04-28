#include <Arduino.h>
#include <Wire.h>
#include <WiFiClientSecure.h>
#include "Mqtt.h"
#include "utils.h"


// Enable or disable Serial comunication  
#define DEBUG 1

// Water flow and water pump
#define WATERFLOW_SIG_PIN 34
#define WATERPUMP_PIN 2
int statePump = 1;

// Functions for ISR
volatile double waterFlow;
#define SQUARE_WAVE 127008
#define LITER 1.0

void pulse()   //measure the quantity of square wave
{
  waterFlow += LITER / SQUARE_WAVE;
}

// Mqtt
WiFiClientSecure espClient;
Mqtt client(espClient);

void setup() {
  Serial.begin(115200);
  if(!DEBUG) Serial.end();
  
  pinMode(WATERPUMP_PIN, OUTPUT);
  digitalWrite(WATERPUMP_PIN, statePump);
  waterFlow = 0;
  attachInterrupt(digitalPinToInterrupt(WATERFLOW_SIG_PIN), pulse, RISING);
  wifiConnect();
}

// Prints
String TextWaterFlow = "...";
String textStatePump = "...";

// Time
unsigned long lastTime = 0;

void loop() {
  // Mqtt connection
  client.reconnect();
  client.loop();

  // Print loop 
  if(millis() > lastTime + 1000){
    lastTime = millis();
    TextWaterFlow = String("waterFlow: " + String(waterFlow) + "L");
    client.publish("sensors/water_flow",TextWaterFlow.c_str());
  
    textStatePump = String("Pump state: " + String(statePump));
    client.publish("sensors/pump_state",textStatePump.c_str());
  }
}
