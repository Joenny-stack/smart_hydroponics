#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define TdsSensorPin 27
#define ONE_WIRE_BUS 32       // DS18B20 on GPIO 32
#define PH_PIN 34             // pH sensor connected to GPIO 34
#define LEVEL 33              // Water level sensor connected to GPIO 33
#define VREF 3.3              // analog reference voltage (Volt) of the ADC
#define SCOUNT 30             // number of samples

int analogBuffer[SCOUNT];
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;

float averageVoltage = 0;
float tdsValue = 0;
float temperature = 25.0;
float pHValue = 7.0;
float levelPercent = 0.0;

// pH calibration slope
float slope = -0.18; // default value; calibrate as needed

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// LCD Setup
LiquidCrystal_I2C lcd(0x27, 16, 2);
unsigned long lastLcdToggle = 0;
bool showSensorData = true;

int getMedianNum(int bArray[], int iFilterLen) {
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++) bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0) {
    bTemp = bTab[(iFilterLen - 1) / 2];
  } else {
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  }
  return bTemp;
}

void setup() {
  Serial.begin(115200);
  pinMode(TdsSensorPin, INPUT);
  pinMode(PH_PIN, INPUT);
  pinMode(LEVEL, INPUT);
  sensors.begin();
  analogReadResolution(12); // For ESP32: 0-4095
  analogSetAttenuation(ADC_11db); // Full range

  // LCD init
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  delay(1000);
  lcd.clear();
}

void loop() {
  static unsigned long analogSampleTimepoint = millis();
  if (millis() - analogSampleTimepoint > 40U) {
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);
    analogBufferIndex++;
    if (analogBufferIndex == SCOUNT) {
      analogBufferIndex = 0;
    }
  }

  static unsigned long printTimepoint = millis();
  if (millis() - printTimepoint > 800U) {
    printTimepoint = millis();

    // Get temperature
    sensors.requestTemperatures();
    temperature = sensors.getTempCByIndex(0);

    // Get pH
    int rawPH = analogRead(PH_PIN);
    float voltagePH = rawPH * (VREF / 4095.0);
    pHValue = 7 + ((voltagePH - 2.5) / slope); // Adjust slope if needed

    // Get Level % (inverted mapping)
    int levelRaw = analogRead(LEVEL);
    levelPercent = map(levelRaw, 4095, 0, 0, 100);

    // Get TDS
    for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++) {
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
    }

    averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 4095.0;
    float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
    float compensationVoltage = averageVoltage / compensationCoefficient;

    tdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage
               - 255.86 * compensationVoltage * compensationVoltage
               + 857.39 * compensationVoltage) * 0.5;

    // Print all sensor data to Serial
    Serial.print("Temp: ");
    Serial.print(temperature);
    Serial.print("°C | pH: ");
    Serial.print(pHValue, 2);
    Serial.print(" | Level: ");
    Serial.print(levelPercent, 0);
    Serial.print("% | TDS: ");
    Serial.print(tdsValue, 0);
    Serial.println(" ppm");
  }

  // LCD toggle every 5 seconds
  if (millis() - lastLcdToggle > 5000) {
    lastLcdToggle = millis();
    showSensorData = !showSensorData;
    lcd.clear();

    if (showSensorData) {
      lcd.setCursor(0, 0);
      lcd.print("T:");
      lcd.print(temperature, 0);
      lcd.print("C pH:");
      lcd.print(pHValue, 1);

      lcd.setCursor(0, 1);
      lcd.print("L:");
      lcd.print(levelPercent, 0);
      lcd.print("% TDS:");
      lcd.print(tdsValue, 0);
    } else {
      lcd.setCursor(0, 0);
      lcd.print("Monitoring...");
      lcd.setCursor(0, 1);
      lcd.print("Sensors Active");
    }
  }
}
