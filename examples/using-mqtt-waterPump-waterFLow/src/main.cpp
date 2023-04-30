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


unsigned long lastTime = 0;

// Functions for ISR
volatile double waterFlow;
volatile int *statePump;
#define SQUARE_WAVE 5880.0
#define LITER 1.0

void pulse()   //measure the quantity of square wave
{
  if(*statePump){
    waterFlow++;
  }
}

// Mqtt
WiFiClientSecure espClient;
Mqtt client(espClient);

void setup() {
  Serial.begin(115200);
  if(!DEBUG) Serial.end();
  
  pinMode(WATERPUMP_PIN, OUTPUT);
  waterFlow = 0;
  attachInterrupt(digitalPinToInterrupt(WATERFLOW_SIG_PIN), pulse, RISING);
  wifiConnect();
  statePump = client.getStatePump();
  digitalWrite(WATERPUMP_PIN, *statePump);
}

// Prints
String TextWaterFlow = "...";
String textStatePump = "...";
String textPulsesPerMinute = "...";
String textPerSecond = "...";

// Time

void loop() {
  // Mqtt connection
  client.reconnect();
  client.loop();

	if(lastTime == 0 && *statePump == 1){
    lastTime = millis();
	}

  // Print loop 
  if(millis() >= lastTime + 60000 && *statePump == 1){
    lastTime = 0;
    textPulsesPerMinute = "Pulses per minute: " + String(waterFlow);
    textPerSecond = "Pulses per second: " + String(waterFlow/60) + "Hz";
    waterFlow = 0;
    *statePump = 0;
    client.publish("sensor/pulses_per_minute", textPulsesPerMinute.c_str());
    client.publish("sensor/pulses_per_second", textPerSecond.c_str());
    /* TextWaterFlow = String("waterFlow: " + String(waterFlow) + "L");
    client.publish("sensors/water_flow",TextWaterFlow.c_str());
  
    textStatePump = String("Pump state: " + String(statePump));
    client.publish("sensors/pump_state",textStatePump.c_str()); */
  }
  digitalWrite(WATERPUMP_PIN, *statePump);
}
