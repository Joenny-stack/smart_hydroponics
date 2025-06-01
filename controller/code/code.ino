#include <WiFiManager.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define TdsSensorPin 35
#define ONE_WIRE_BUS 32
#define PH_PIN 34
#define LEVEL 33
#define PUMP_PIN 26
#define VREF 3.3
#define SCOUNT 30

int analogBuffer[SCOUNT];
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;
bool tdsBufferReady = false;

float averageVoltage = 0;
float tdsValue = 0;
float temperature = 25.0;
float pHValue = 7.0;
float levelPercent = 0.0;
float slope = -0.18;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

LiquidCrystal_I2C lcd(0x27, 16, 2);
unsigned long lastLcdToggle = 0;
bool showSensorData = true;

WebServer server(80);

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

void handleStatus() {
  String json = "{";
  json += "\"temperature\":" + String(temperature, 2) + ",";
  json += "\"ph\":" + String(pHValue, 2) + ",";
  json += "\"water_level\":" + String(levelPercent, 2) + ",";
  json += "\"tds\":" + String(tdsValue, 2) + ",";
  json += "\"pump\":" + String(digitalRead(PUMP_PIN));
  json += "}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(TdsSensorPin, INPUT);
  pinMode(PH_PIN, INPUT);
  pinMode(LEVEL, INPUT);
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);

  sensors.begin();
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi...");

  WiFi.mode(WIFI_STA);
  WiFiManager wm;
  wm.setConfigPortalTimeout(180);
  if (!wm.autoConnect("Greenhouse_Setup")) {
    Serial.println("❌ WiFi failed. Restarting...");
    ESP.restart();
  }

  Serial.print("✅ Connected! IP: ");
  Serial.println(WiFi.localIP());

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IP:");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());

  server.on("/status", handleStatus);
  server.begin();

  Serial.println("Filling TDS buffer...");
  for (int i = 0; i < SCOUNT; i++) {
    analogBuffer[i] = analogRead(TdsSensorPin);
    delay(40);
  }
  tdsBufferReady = true;
  Serial.println("TDS buffer ready.");
}

void loop() {
  server.handleClient();

  static unsigned long analogSampleTimepoint = millis();
  if (millis() - analogSampleTimepoint > 40U) {
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);
    analogBufferIndex++;
    if (analogBufferIndex == SCOUNT) {
      analogBufferIndex = 0;
      tdsBufferReady = true;
    }
  }

  static unsigned long printTimepoint = millis();
  if (millis() - printTimepoint > 800U) {
    printTimepoint = millis();

    sensors.requestTemperatures();
    temperature = sensors.getTempCByIndex(0);

    int rawPH = analogRead(PH_PIN);
    float voltagePH = rawPH * (VREF / 4095.0);
    pHValue = 7 + ((voltagePH - 2.1) / slope);

    int levelRaw = analogRead(LEVEL);
    levelPercent = map(levelRaw, 4095, 0, 0, 100);

    if (tdsBufferReady) {
      for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++) {
        analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
      }

      int rawMedian = getMedianNum(analogBufferTemp, SCOUNT);
      averageVoltage = rawMedian * VREF / 4095.0;
      float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
      float compensationVoltage = averageVoltage / compensationCoefficient;

      tdsValue = (133.42 * pow(compensationVoltage, 3)
                - 255.86 * pow(compensationVoltage, 2)
                + 857.39 * compensationVoltage) * 0.5;

      Serial.printf("TDS: %.0f ppm | Raw ADC: %d | Voltage: %.3f V\n", tdsValue, rawMedian, averageVoltage);
    }

    if (levelPercent < 70 ) {
      digitalWrite(PUMP_PIN, HIGH);
      Serial.println("Pump ON");
    } else {
      digitalWrite(PUMP_PIN, LOW);
      Serial.println("Pump OFF");
    }

    Serial.printf("Temp: %.2f°C | pH: %.2f | Level: %.0f%%\n", temperature, pHValue, levelPercent);
  }

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
      lcd.print("Monitoring:IP");
      lcd.setCursor(0, 1);
      lcd.print(WiFi.localIP());
    }
  }
}
