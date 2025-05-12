import 'dart:async';
import 'dart:convert';
import 'dart:developer'; // 👈 Import the logging framework
import 'package:http/http.dart' as http;
import '../models/device_status.dart';

class ApiService {
  static Stream<DeviceStatus> fetchStatusStream(String espIp) async* {
    while (true) {
      try {
        final response = await http
            .get(Uri.parse('http://$espIp/status'))
            .timeout(const Duration(seconds: 10)); // 👈 Add timeout

        if (response.statusCode == 200) {
          final json = jsonDecode(response.body);
          yield DeviceStatus.fromJson(json); // 👈 Yield valid data
        } else {
          yield* Stream.error('Error: Received status code ${response.statusCode}'); // 👈 Yield an error
        }
      } catch (e) {
        log('Error: $e', error: e); // 👈 Log the error
        yield* Stream.error('Failed to connect to the board. Please check the IP address.'); // 👈 Propagate error
      }

      await Future.delayed(const Duration(seconds: 1)); // 👈 Continue fetching
    }
  }
}
