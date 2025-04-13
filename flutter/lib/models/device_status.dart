class DeviceStatus {
  final double temperature;
  final double ph;
  final double soilMoisture;

  DeviceStatus({
    required this.temperature,
    required this.ph,
    required this.soilMoisture,
  });

  factory DeviceStatus.fromJson(Map<String, dynamic> json) {
    return DeviceStatus(
      temperature: json['temperature'].toDouble(),
      ph: json['ph'].toDouble(),
      soilMoisture: json['soil_moisture'].toDouble(),
    );
  }
}
