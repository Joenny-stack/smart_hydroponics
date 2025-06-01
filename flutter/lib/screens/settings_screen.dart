import 'package:flutter/material.dart';
import '../services/readings_db.dart'; // Import ReadingsDB

class SettingsScreen extends StatefulWidget {
  final String currentIp;
  final void Function(String) onIpChanged;
  const SettingsScreen({Key? key, required this.currentIp, required this.onIpChanged}) : super(key: key);

  @override
  State<SettingsScreen> createState() => _SettingsScreenState();
}

class _SettingsScreenState extends State<SettingsScreen> {
  late TextEditingController _ipController;

  @override
  void initState() {
    super.initState();
    _ipController = TextEditingController(text: widget.currentIp);
  }

  @override
  void dispose() {
    _ipController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('Settings')),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            const Text('Current System IP Address:', style: TextStyle(fontSize: 16, fontWeight: FontWeight.bold)),
            const SizedBox(height: 8),
            Text(widget.currentIp, style: const TextStyle(fontSize: 16)),
            const SizedBox(height: 24),
            TextField(
              controller: _ipController,
              decoration: const InputDecoration(
                labelText: 'Change System IP Address',
                border: OutlineInputBorder(),
              ),
              keyboardType: TextInputType.number,
            ),
            const SizedBox(height: 16),
            ElevatedButton(
              onPressed: () async {
                final newIp = _ipController.text;
                widget.onIpChanged(newIp);
                await ReadingsDB().saveLastUsedIp(newIp);
                if (context.mounted) {
                  Navigator.pop(context, newIp); // Return the new IP to the previous screen
                }
              },
              child: const Text('Save'),
            ),
          ],
        ),
      ),
    );
  }
}
