import 'package:flutter/material.dart';
import '../services/readings_db.dart';

class ReadingsScreen extends StatefulWidget {
  const ReadingsScreen({super.key});

  @override
  State<ReadingsScreen> createState() => _ReadingsScreenState();
}

class _ReadingsScreenState extends State<ReadingsScreen> {
  late Future<List<Map<String, dynamic>>> _readingsFuture;

  @override
  void initState() {
    super.initState();
    _readingsFuture = ReadingsDB.getReadings();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('Readings History')),
      body: FutureBuilder<List<Map<String, dynamic>>>(
        future: _readingsFuture,
        builder: (context, snapshot) {
          if (snapshot.connectionState == ConnectionState.waiting) {
            return const Center(child: CircularProgressIndicator());
          } else if (snapshot.hasData && snapshot.data!.isNotEmpty) {
            final readings = snapshot.data!;
            return ListView.builder(
              itemCount: readings.length,
              itemBuilder: (context, index) {
                final r = readings[index];
                return Card(
                  margin: const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
                  child: ListTile(
                    title: Text('Time: ${r['timestamp']}'),
                    subtitle: Column(
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: [
                        Text('Temperature: ${r['temperature']} °C'),
                        Text('pH: ${r['ph']}'),
                        Text('Soil Moisture: ${r['soilMoisture']}%'),
                        Text('Water Level: ${r['waterLevel']}%'),
                        Text('TDS: ${r['tds']} ppm'),
                      ],
                    ),
                  ),
                );
              },
            );
          } else {
            return const Center(child: Text('No readings stored yet.'));
          }
        },
      ),
    );
  }
}
