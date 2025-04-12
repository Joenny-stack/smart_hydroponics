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

// Function to handle HTTP GET request
void handleSensorData() {
  // Read pH
  int rawPH = analogRead(PH_PIN);
  float voltagePH = rawPH * (3.3 / 4095.0);
  float pH = 7 + ((voltagePH - 2.5) / slope);

  // Read Temperature
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);

  // Read Soil Moisture
  int soilRaw = analogRead(SOIL_MOIST);
  float soilPercent = map(soilRaw, 4095, 0, 0, 100);

  // Send JSON response
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

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected! IP address: ");
  Serial.println(WiFi.localIP());

  // Set up route
  server.on("/", handleSensorData);
  server.begin();
}

void loop() {
  server.handleClient();
}
