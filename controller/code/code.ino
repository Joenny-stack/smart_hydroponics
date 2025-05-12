#include <WiFiManager.h>               // Wi-Fi Manager
#include <WebServer.h>
#include <OneWire.h>                   // For temp sensor (not used here)
#include <DallasTemperature.h>        // For temp sensor (not used here)

// Pin definitions
#define PH_PIN 34
#define LEVEL 33
#define TDS_PIN 27  // Correct pin for TDS

// Web server
WebServer server(80);

// pH calibration slope
float slope = -0.18;

// TDS config
#define VREF 3.3       // Analog reference voltage
#define SCOUNT 30      // Sample count for median filtering

void handleSensorData() {
  Serial.println("=== Reading Sensors ===");

  // === pH Sensor ===
  int rawPH = analogRead(PH_PIN);
  float voltagePH = rawPH * (3.3 / 4095.0);
  float pH = 7 + ((voltagePH - 2.5) / slope);
  Serial.print("Raw pH: ");
  Serial.println(rawPH);
  Serial.print("Voltage pH: ");
  Serial.println(voltagePH);
  Serial.print("Calculated pH: ");
  Serial.println(pH);

  // === Water Level ===
  int levelRaw = analogRead(LEVEL);
  float levelPercent = map(levelRaw, 4095, 0, 0, 100);
  Serial.print("Water Level Raw: ");
  Serial.println(levelRaw);
  Serial.print("Water Level %: ");
  Serial.println(levelPercent);

  // === TDS Sensor Reading ===
  int tdsSamples[SCOUNT];
  for (int i = 0; i < SCOUNT; i++) {
    tdsSamples[i] = analogRead(TDS_PIN);
    delay(10);  // Required for stable ADC reads
  }

  // Sort to get median
  for (int i = 0; i < SCOUNT - 1; i++) {
    for (int j = i + 1; j < SCOUNT; j++) {
      if (tdsSamples[i] > tdsSamples[j]) {
        int temp = tdsSamples[i];
        tdsSamples[i] = tdsSamples[j];
        tdsSamples[j] = temp;
      }
    }
  }

  float avgRaw = tdsSamples[SCOUNT / 2];  // Median
  float avgVoltage = avgRaw * (VREF / 4095.0);
  float temperatureC = 25.0;  // Hardcoded temperature for TDS
  float compensationCoefficient = 1.0 + 0.02 * (temperatureC - 25.0);
  float compensatedVoltage = avgVoltage / compensationCoefficient;
  float tdsValue = (133.42 * compensatedVoltage * compensatedVoltage * compensatedVoltage
                    - 255.86 * compensatedVoltage * compensatedVoltage
                    + 857.39 * compensatedVoltage) * 0.5;

  Serial.print("TDS Raw (median): ");
  Serial.println(avgRaw);
  Serial.print("TDS Voltage: ");
  Serial.println(avgVoltage, 3);
  Serial.print("TDS (ppm): ");
  Serial.println(tdsValue, 2);

  // === JSON Response ===
  String json = "{";
  json += "\"temperature\":25.00,";
  json += "\"ph\":" + String(pH, 2) + ",";
  json += "\"water_level\":" + String(levelPercent, 2) + ",";
  json += "\"tds\":" + String(tdsValue, 2);
  json += "}";

  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Booting...");

  WiFi.mode(WIFI_STA);
  WiFiManager wm;

  if (!wm.autoConnect("Greenhouse_Setup")) {
    Serial.println("❌ Failed to connect to WiFi. Restarting...");
    ESP.restart();
  }

  Serial.println("✅ WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Web server started.");

  server.on("/status", handleSensorData);
  server.begin();
}

void loop() {
  server.handleClient();
}
