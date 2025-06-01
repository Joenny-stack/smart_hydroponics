import 'package:flutter/material.dart';
import 'dart:async';
import '../services/readings_db.dart';

class ReadingsScreen extends StatefulWidget {
  const ReadingsScreen({super.key});

  @override
  State<ReadingsScreen> createState() => _ReadingsScreenState();
}

class _ReadingsScreenState extends State<ReadingsScreen> {
  List<Map<String, dynamic>> _readings = [];
  DateTime? _selectedDate;
  Timer? _pollTimer;

  @override
  void initState() {
    super.initState();
    _loadReadings();
    _startPolling();
  }

  void _loadReadings() async {
    final allReadings = await ReadingsDB.getReadings();
    setState(() {
      if (_selectedDate == null) {
        _readings = allReadings;
      } else {
        _readings = allReadings.where((r) =>
          DateTime.parse(r['timestamp']).year == _selectedDate!.year &&
          DateTime.parse(r['timestamp']).month == _selectedDate!.month &&
          DateTime.parse(r['timestamp']).day == _selectedDate!.day
        ).toList();
      }
    });
  }

  void _startPolling() {
    _pollTimer?.cancel();
    _pollTimer = Timer.periodic(const Duration(seconds: 30), (_) {
      _loadReadings();
    });
  }

  void _filterByDate(DateTime? date) {
    setState(() {
      _selectedDate = date;
    });
    _loadReadings();
  }

  @override
  void dispose() {
    _pollTimer?.cancel();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('Readings History')),
      body: Column(
        children: [
          Padding(
            padding: const EdgeInsets.all(8.0),
            child: Row(
              children: [
                Expanded(
                  child: OutlinedButton.icon(
                    icon: const Icon(Icons.calendar_today),
                    label: Text(_selectedDate == null
                        ? 'Filter by Date'
                        : '${_selectedDate!.year}-${_selectedDate!.month.toString().padLeft(2, '0')}-${_selectedDate!.day.toString().padLeft(2, '0')}'),
                    onPressed: () async {
                      final picked = await showDatePicker(
                        context: context,
                        initialDate: _selectedDate ?? DateTime.now(),
                        firstDate: DateTime(2020),
                        lastDate: DateTime.now(),
                      );
                      _filterByDate(picked);
                    },
                  ),
                ),
                if (_selectedDate != null)
                  IconButton(
                    icon: const Icon(Icons.clear),
                    tooltip: 'Clear filter',
                    onPressed: () => _filterByDate(null),
                  ),
              ],
            ),
          ),
          Expanded(
            child: _readings.isEmpty
                ? const Center(child: Text('No readings stored yet.'))
                : SingleChildScrollView(
                    scrollDirection: Axis.vertical,
                    child: SingleChildScrollView(
                      scrollDirection: Axis.horizontal,
                      child: DataTable(
                        columns: const [
                          DataColumn(label: Text('Time')),
                          DataColumn(label: Text('Temp (°C)')),
                          DataColumn(label: Text('pH')),
                          DataColumn(label: Text('Water Level (%)')),
                          DataColumn(label: Text('TDS (ppm)')),
                        ],
                        rows: _readings.map((r) => DataRow(cells: [
                          DataCell(Text(r['timestamp'].toString().replaceFirst('T', '\n').substring(0, 16))),
                          DataCell(Text(r['temperature'].toString())),
                          DataCell(Text(r['ph'].toString())),
                          DataCell(Text(r['waterLevel'].toString())),
                          DataCell(Text(r['tds'].toString())),
                        ])).toList(),
                      ),
                    ),
                  ),
          ),
        ],
      ),
    );
  }
}
