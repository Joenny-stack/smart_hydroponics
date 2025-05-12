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
  Stream<DeviceStatus> _statusStream = Stream.empty(); // 👈 Initialize with an empty stream
  String espIp = ''; // 👈 Initialize as empty string

  @override
  void initState() {
    super.initState();
    WidgetsBinding.instance.addPostFrameCallback((_) {
      _showIpDialog();
    });
  }

  void _showIpDialog() async {
    final ipController = TextEditingController();
    final enteredIp = await showDialog<String>(
      context: context,
      builder: (context) {
        return AlertDialog(
          title: const Text("Enter ESP IP Address"),
          content: TextField(
            controller: ipController,
            decoration: const InputDecoration(
              hintText: "e.g., 192.168.1.100",
            ),
            keyboardType: TextInputType.number,
          ),
          actions: [
            TextButton(
              onPressed: () => Navigator.of(context).pop(),
              child: const Text("Cancel"),
            ),
            TextButton(
              onPressed: () => Navigator.of(context).pop(ipController.text),
              child: const Text("Save"),
            ),
          ],
        );
      },
    );

    if (enteredIp != null && enteredIp.isNotEmpty) {
      setState(() {
        espIp = enteredIp;
        _statusStream = ApiService.fetchStatusStream(espIp); // 👈 Update the stream
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text("Smart Hydroponics")),
      body: StreamBuilder<DeviceStatus>(
        stream: _statusStream,
        builder: (context, snapshot) {
          if (snapshot.connectionState == ConnectionState.waiting) {
            return const Center(child: CircularProgressIndicator());
          } else if (snapshot.hasData) {
            final data = snapshot.data!;
            return SingleChildScrollView(
              padding: const EdgeInsets.all(16),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Card(
                    elevation: 4,
                    margin: const EdgeInsets.only(bottom: 16),
                    child: Padding(
                      padding: const EdgeInsets.all(16),
                      child: Column(
                        crossAxisAlignment: CrossAxisAlignment.start,
                        children: [
                          const Text(
                            "Status",
                            style: TextStyle(
                              fontSize: 20,
                              fontWeight: FontWeight.bold,
                            ),
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
                            value: data.ph.toStringAsFixed(2),
                            icon: Icons.science,
                            color: Colors.orange,
                          ),
                          StatusTile(
                            label: "Soil Moisture",
                            value: "${data.soilMoisture.toStringAsFixed(2)}%",
                            icon: Icons.grass,
                            color: Colors.green,
                          ),
                          StatusTile(
                            label: "Water Level",
                            value: "${data.waterLevel.toStringAsFixed(2)}%",
                            icon: Icons.water,
                            color: Colors.blue,
                          ),
                          StatusTile(
                            label: "TDS",
                            value: "${data.tds.toStringAsFixed(2)} ppm",
                            icon: Icons.opacity,
                            color: Colors.purple,
                          ),
                        ],
                      ),
                    ),
                  ),
                  SizedBox(
                    width: double.infinity, // 👈 Make the alerts card fill the width
                    child: Card(
                      elevation: 4,
                      child: Padding(
                        padding: const EdgeInsets.all(16),
                        child: Column(
                          crossAxisAlignment: CrossAxisAlignment.start,
                          children: [
                            const Text(
                              "Alerts",
                              style: TextStyle(
                                fontSize: 20,
                                fontWeight: FontWeight.bold,
                              ),
                            ),
                            const SizedBox(height: 16),
                            if (data.checkAlerts().isEmpty)
                              const Text(
                                "No alerts at the moment.",
                                style: TextStyle(fontSize: 16),
                              )
                            else
                              ...data.checkAlerts().map((alert) => ListTile(
                                    leading: const Icon(
                                      Icons.warning,
                                      color: Colors.red,
                                    ), // 👈 Add an icon for each alert
                                    title: Text(
                                      alert,
                                      style: const TextStyle(
                                        fontSize: 16,
                                        color: Colors.black,
                                      ),
                                    ),
                                  )),
                          ],
                        ),
                      ),
                    ),
                  ),
                ],
              ),
            );
          } else if (snapshot.hasError) {
            return Center(
              child: Column(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  const Icon(Icons.error, color: Colors.red, size: 64),
                  const SizedBox(height: 16),
                  const Text(
                    "Connection Error",
                    style: TextStyle(
                      fontSize: 20,
                      fontWeight: FontWeight.bold,
                      color: Colors.red,
                    ),
                  ),
                  const SizedBox(height: 8),
                  Text(
                    "Failed to connect to the board.\n"
                    "This could be due to:\n"
                    "- An incorrect IP address\n"
                    "- The board and device not being on the same network\n\n"
                    "Please check and try again.",
                    textAlign: TextAlign.center,
                    style: const TextStyle(fontSize: 16, color: Colors.black),
                  ),
                  const SizedBox(height: 16),
                  ElevatedButton(
                    onPressed: _showIpDialog, // 👈 Allow retry by showing the IP dialog again
                    child: const Text("Retry"),
                  ),
                ],
              ),
            );
          }
          return const Center(child: Text("No data available")); // 👈 Handle no data case
        },
      ),
    );
  }
}
