import '../services/notification_service.dart';

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
      String alert = "Temperature is out of range (15-30 °C).";
      alerts.add(alert);
      NotificationService.showNotification("Alert", alert);
    }
    if (ph < 5.5 || ph > 7.5) {
      String alert = "pH level is out of range (5.5-7.5).";
      alerts.add(alert);
      NotificationService.showNotification("Alert", alert);
    }
    if (soilMoisture < 20 || soilMoisture > 80) {
      String alert = "Soil moisture is out of range (20-80%).";
      alerts.add(alert);
      NotificationService.showNotification("Alert", alert);
    }
    if (waterLevel < 50) {
      String alert = "Water level is too low (<50%).";
      alerts.add(alert);
      NotificationService.showNotification("Alert", alert);
    }
    if (tds < 500 || tds > 1500) {
      String alert = "TDS is out of range (500-1500 ppm).";
      alerts.add(alert);
      NotificationService.showNotification("Alert", alert);
    }
    return alerts;
  }
}
