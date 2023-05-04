#include <Arduino.h>
#include <Ticker.h>
#include <Wire.h>
#include <WiFiClientSecure.h>
#include "Mqtt.h"
#include <ArduinoJson.h>

/*
  Application:
  - Interface water flow sensor with ESP32 board.

  Board:
  - ESP32 Dev Module
    https://my.cytron.io/p-node32-lite-wifi-and-bluetooth-development-kit

  Sensor:
  - G 1/2 Water Flow Sensor
    https://my.cytron.io/p-g-1-2-water-flow-sensor
 */

#define WATERPUMP_PIN 2
#define SENSOR 34

long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
bool ledState = LOW;
volatile int pulseCount;
int pulse1Sec = 0;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalLitres;

Ticker waterPump;

void IRAM_ATTR pulseCounter()
{
  pulseCount++;
}

WiFiClientSecure espClient;
Mqtt client(espClient);

double &calibrationFactor = client.getCalibrationFactor();
int &statePump = client.getStatePump();

void setup()
{
  Serial.begin(115200);

  randomSeed(analogRead(15));

  pinMode(WATERPUMP_PIN, OUTPUT);
  pinMode(SENSOR, INPUT_PULLUP);

  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalLitres = 0;
  previousMillis = 0;

  attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, FALLING);
  client.wifiConnect();
}

// Printspio

char bufferFlowRate[512];
char bufferTotalLitres[512];

void loop()
{
  client.reconnect();
  client.loop();

  digitalWrite(WATERPUMP_PIN, statePump);

  if(!statePump){
    totalLitres = 0;
    previousMillis = 0;
  }

  currentMillis = millis();
  if (currentMillis - previousMillis > interval)
  {

    pulse1Sec = pulseCount;
    pulseCount = 0;

    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming from the sensor.
    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();

    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    flowMilliLitres = (flowRate / 60); // (flowRate / 60) * 1000

    // Add the millilitres passed in this second to the cumulative total
    totalLitres += flowMilliLitres;

    StaticJsonDocument<200> ObjtFlowRate;
    ObjtFlowRate["unit"] = "mL/min";
    ObjtFlowRate["value"] = flowRate;

    StaticJsonDocument<200> ObjtTotalLitres;
    ObjtTotalLitres["unit"] = "mL";
    ObjtTotalLitres["value"] = totalLitres;

    serializeJson(ObjtFlowRate, bufferFlowRate);
    serializeJson(ObjtTotalLitres, bufferTotalLitres);

    client.publish("sensor/water_flow/flow_rate", bufferFlowRate);
    client.publish("sensor/water_flow/liquid_volume", bufferTotalLitres);
  }
}