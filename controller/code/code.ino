#include <WiFi.h>
#include <WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Wi-Fi credentials
const char* ssid = "Boardroom";
const char* password = "board@2023";

// Pin definitions
#define PH_PIN 34
#define TEMP_PIN 32
#define SOIL_MOIST 33

// Sensor objects
OneWire oneWire(TEMP_PIN);
DallasTemperature sensors(&oneWire);

// Web server runs on port 80
WebServer server(80);

// pH calibration slope
float slope = -0.18;

// Handle HTTP GET
void handleSensorData() {
  int rawPH = analogRead(PH_PIN);
  float voltagePH = rawPH * (3.3 / 4095.0);
  float pH = 7 + ((voltagePH - 2.5) / slope);

  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);

  int soilRaw = analogRead(SOIL_MOIST);
  float soilPercent = map(soilRaw, 4095, 0, 0, 100);

  String json = "{";
  json += "\"temperature\":" + String(temperatureC, 2) + ",";
  json += "\"ph\":" + String(pH, 2) + ",";
  json += "\"soil_moisture\":" + String(soilPercent, 2);
  json += "}";

  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  sensors.begin();

  WiFi.mode(WIFI_STA); // Ensure it's in station mode
  WiFi.begin(ssid, password);

  Serial.println("Connecting to WiFi...");
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(1000);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    server.on("/", handleSensorData);
    server.begin();
  } else {
    Serial.println("\nFailed to connect to WiFi.");
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    server.handleClient();
  }
}
