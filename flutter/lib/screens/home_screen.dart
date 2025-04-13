import 'package:flutter/material.dart';
import '../models/device_status.dart';
import '../services/api_service.dart';
import '../widgets/status_tile.dart';

class HomeScreen extends StatefulWidget {
  const HomeScreen({super.key});

  @override
  State<HomeScreen> createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen> {
  late Stream<DeviceStatus> _statusStream;
  final String espIp = '10.91.178.69'; // 👈 Replace with your ESP IP

  @override
  void initState() {
    super.initState();
    _statusStream = ApiService.fetchStatusStream(espIp);
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text("Smart Hydroponics")),
      body: StreamBuilder<DeviceStatus>(
        stream: _statusStream,
        builder: (context, snapshot) {
          if (snapshot.hasData) {
            final data = snapshot.data!;
            return SingleChildScrollView(
              padding: const EdgeInsets.all(16),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  const Text(
                    "Status",
                    style: TextStyle(fontSize: 20, fontWeight: FontWeight.bold),
                  ),
                  const SizedBox(height: 16),
                  StatusTile(
                    label: "Temperature",
                    value: "${data.temperature.toStringAsFixed(2)} °C",
                    icon: Icons.thermostat,
                    color: Colors.red,
                  ),
                  StatusTile(
                    label: "pH Level",
                    value: "${data.ph.toStringAsFixed(2)}",
                    icon: Icons.science,
                    color: Colors.orange,
                  ),
                  StatusTile(
                    label: "Soil Moisture",
                    value: "${data.soilMoisture.toStringAsFixed(2)}%",
                    icon: Icons.grass,
                    color: Colors.green,
                  ),
                ],
              ),
            );
          } else if (snapshot.hasError) {
            return Center(child: Text("Error: ${snapshot.error}"));
          }
          return const Center(child: CircularProgressIndicator());
        },
      ),
    );
  }
}
