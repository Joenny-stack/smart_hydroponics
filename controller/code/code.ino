#define PH_PIN 34           // pH sensor
#define TEMP_PIN 32         // DS18B20 temperature sensor
#define SOIL_MOIST 33       // Soil moisture sensor

#include <OneWire.h>
#include <DallasTemperature.h>

OneWire oneWire(TEMP_PIN);
DallasTemperature sensors(&oneWire);

float slope = -0.18; // Adjust this from your pH calibration

void setup() {
  Serial.begin(115200);
  sensors.begin();  // Initialize temperature sensor
}

void loop() {
  // --- pH Sensor Reading ---
  int rawPH = analogRead(PH_PIN);
  float voltagePH = rawPH * (3.3 / 4095.0);
  float pH = 7 + ((voltagePH - 2.5) / slope);

  // --- Temperature Reading ---
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);

  // --- Soil Moisture Reading ---
  int soilRaw = analogRead(SOIL_MOIST);
  float soilPercent = map(soilRaw, 4095, 0, 0, 100); // dry = high voltage

  // --- Display All Readings ---

  Serial.print("pH: ");
  Serial.print(pH, 2);
  Serial.print(" | Temp: ");
  Serial.print(temperatureC);
  Serial.print(" °C | Soil Moisture: ");
  Serial.print(soilPercent);
  Serial.println(" %");


  delay(1000);
}
