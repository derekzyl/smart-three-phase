import 'dart:async';
import 'dart:convert';

import 'package:http/http.dart' as http;
import 'package:web_socket_channel/web_socket_channel.dart';

import '../models/telemetry.dart';

class ApiService {
  ApiService({String? baseUrl})
      : baseUrl = baseUrl ??
            const String.fromEnvironment(
              'API_BASE',
              defaultValue: 'http://127.0.0.1:8000',
            );

  final String baseUrl;
  WebSocketChannel? _ws;
  StreamSubscription? _wsSub;

  Uri _u(String path) => Uri.parse('$baseUrl$path');

  Future<Telemetry?> latest() async {
    final r = await http.get(_u('/api/v1/telemetry/latest'));
    if (r.statusCode == 200 && r.body.isNotEmpty && r.body != 'null') {
      return Telemetry.fromJson(jsonDecode(r.body) as Map<String, dynamic>);
    }
    return null;
  }

  Future<List<FaultEvent>> events({int limit = 50}) async {
    final r = await http.get(_u('/api/v1/events?limit=$limit'));
    if (r.statusCode != 200) return [];
    final list = jsonDecode(r.body) as List<dynamic>;
    return list
        .map((e) => FaultEvent.fromJson(e as Map<String, dynamic>))
        .toList();
  }

  Future<bool> reset({required String token, String deviceId = 'PG-PROTO-001'}) async {
    final r = await http.post(
      _u('/api/v1/command/reset'),
      headers: {'Content-Type': 'application/json'},
      body: jsonEncode({'token': token, 'device_id': deviceId}),
    );
    return r.statusCode == 200;
  }

  Future<Telemetry?> injectSim({
    double va = 230,
    double vb = 230,
    double vc = 230,
    double ia = 4,
    double ib = 4,
    double ic = 4,
    String fault = 'NONE',
    String relay = 'CLOSED',
  }) async {
    final r = await http.post(
      _u('/api/v1/sim/telemetry'),
      headers: {'Content-Type': 'application/json'},
      body: jsonEncode({
        'id': 'PG-PROTO-001',
        'Va': va,
        'Vb': vb,
        'Vc': vc,
        'Ia': ia,
        'Ib': ib,
        'Ic': ic,
        'fault': fault,
        'relay': relay,
      }),
    );
    if (r.statusCode == 200) {
      return Telemetry.fromJson(jsonDecode(r.body) as Map<String, dynamic>);
    }
    return null;
  }

  /// Live stream; callback receives telemetry maps (ignores ping).
  void connectLive(void Function(Telemetry t) onData, {void Function(Object e)? onError}) {
    final wsBase = baseUrl.replaceFirst('http://', 'ws://').replaceFirst('https://', 'wss://');
    _ws = WebSocketChannel.connect(Uri.parse('$wsBase/api/v1/ws/live'));
    _wsSub = _ws!.stream.listen(
      (raw) {
        try {
          final map = jsonDecode(raw as String) as Map<String, dynamic>;
          if (map['type'] == 'ping') return;
          if (map.containsKey('Va') || map.containsKey('va') || map.containsKey('device_id')) {
            onData(Telemetry.fromJson(map));
          }
        } catch (e) {
          onError?.call(e);
        }
      },
      onError: onError,
    );
  }

  void dispose() {
    _wsSub?.cancel();
    _ws?.sink.close();
  }
}
