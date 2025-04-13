#include <WiFiManager.h>               // Wi-Fi Manager
#include <WebServer.h>
#include <OneWire.h>  //for temp sensor
#include <DallasTemperature.h> //for temp sensor

// Pin definitions
#define PH_PIN 34
#define TEMP_PIN 32
#define LEVEL 33

// Sensor objects
OneWire oneWire(TEMP_PIN);
DallasTemperature sensors(&oneWire);

// Web server
WebServer server(80);

// pH calibration slope
float slope = -0.18;

void handleSensorData() {
  int rawPH = analogRead(PH_PIN);
  float voltagePH = rawPH * (3.3 / 4095.0);
  float pH = 7 + ((voltagePH - 2.5) / slope);

  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);

  int levelRaw = analogRead(LEVEL);
  float levelPercent = map(levelRaw, 4095, 0, 0, 100);

  String json = "{";
  json += "\"temperature\":" + String(temperatureC, 2) + ",";
  json += "\"ph\":" + String(pH, 2) + ",";
  json += "\"water_level\":" + String(levelPercent, 2);
  json += "}";

  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  sensors.begin();

  WiFi.mode(WIFI_STA);
  WiFiManager wm;

  // Automatically connect to saved Wi-Fi or launch AP if it fails
  if (!wm.autoConnect("Greenhouse_Setup")) {
    Serial.println("Failed to connect and no saved credentials.");
    // Optionally reset or halt
    ESP.restart();
  }

  Serial.println("WiFi connected!");
  Serial.println(WiFi.localIP());

  server.on("/status", handleSensorData);
  server.begin();
}

void loop() {
  server.handleClient();
}
