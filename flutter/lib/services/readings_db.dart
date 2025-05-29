import 'package:sqflite/sqflite.dart';
import 'package:path/path.dart';
import '../models/device_status.dart';

class ReadingsDB {
  static Database? _db;

  static Future<Database> get database async {
    if (_db != null) return _db!;
    _db = await _initDb();
    return _db!;
  }

  static Future<Database> _initDb() async {
    final dbPath = await getDatabasesPath();
    final path = join(dbPath, 'readings.db');
    return await openDatabase(
      path,
      version: 1,
      onCreate: (db, version) async {
        await db.execute('''
          CREATE TABLE readings(
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp TEXT,
            temperature REAL,
            ph REAL,
            soilMoisture REAL,
            waterLevel REAL,
            tds REAL
          )
        ''');
      },
    );
  }

  static Future<void> insertReading(DeviceStatus status) async {
    final db = await database;
    await db.insert('readings', {
      'timestamp': DateTime.now().toIso8601String(),
      'temperature': status.temperature,
      'ph': status.ph,
      'soilMoisture': status.soilMoisture,
      'waterLevel': status.waterLevel,
      'tds': status.tds,
    });
  }

  static Future<List<Map<String, dynamic>>> getReadings() async {
    final db = await database;
    return db.query('readings', orderBy: 'timestamp DESC');
  }


  Future<void> saveLastUsedIp(String ip) async {
    final db = await database;
    await db.execute('''
      CREATE TABLE IF NOT EXISTS settings (
        key TEXT PRIMARY KEY,
        value TEXT
      )
    ''');
    await db.insert(
      'settings',
      {'key': 'last_ip', 'value': ip},
      conflictAlgorithm: ConflictAlgorithm.replace,
    );
  }

  Future<String?> getLastUsedIp() async {
    final db = await database;
    await db.execute('''
      CREATE TABLE IF NOT EXISTS settings (
        key TEXT PRIMARY KEY,
        value TEXT
      )
    ''');
    final result = await db.query('settings', where: 'key = ?', whereArgs: ['last_ip']);
    if (result.isNotEmpty) {
      return result.first['value'] as String?;
    }
    return null;
  }
}

