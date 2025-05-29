class DeviceStatus {
  final double temperature;
  final double ph;
  final double soilMoisture;
  final double waterLevel;
  final double tds;

  DeviceStatus({
    required this.temperature,
    required this.ph,
    required this.soilMoisture,
    required this.waterLevel,
    required this.tds,
  });

  factory DeviceStatus.fromJson(Map<String, dynamic> json) {
    return DeviceStatus(
      temperature: (json['temperature'] ?? 0).toDouble(),
      ph: (json['ph'] ?? 0).toDouble(),
      soilMoisture: (json['soil_moisture'] ?? 0).toDouble(),
      waterLevel: (json['water_level'] ?? 0).toDouble(),
      tds: (json['tds'] ?? 0).toDouble(),
    );
  }

  List<String> checkAlerts() {
    List<String> alerts = [];
    if (temperature < 15 || temperature > 30) {
      alerts.add("Temperature is out of range (15-30 °C).");
    }
    if (ph < 5.5 || ph > 7.5) {
      alerts.add("pH level is out of range (5.5-7.5).");
    }
    if (soilMoisture < 20 || soilMoisture > 80) {
      alerts.add("Soil moisture is out of range (20-80%).");
    }
    if (waterLevel < 50) {
      alerts.add("Water level is too low (<50%).");
    }
    if (tds < 500 || tds > 1500) {
      alerts.add("TDS is out of range (500-1500 ppm).");
    }
    return alerts;
  }
}
