class Telemetry {
  Telemetry({
    required this.deviceId,
    required this.ts,
    required this.va,
    required this.vb,
    required this.vc,
    required this.ia,
    required this.ib,
    required this.ic,
    required this.fault,
    required this.relay,
  });

  final String deviceId;
  final DateTime ts;
  final double va, vb, vc, ia, ib, ic;
  final String fault;
  final String relay;

  bool get isHealthy => fault.toUpperCase() == 'NONE' && relay.toUpperCase() == 'CLOSED';

  factory Telemetry.fromJson(Map<String, dynamic> j) {
    final tsRaw = j['ts'];
    DateTime ts;
    if (tsRaw is String) {
      ts = DateTime.tryParse(tsRaw)?.toLocal() ?? DateTime.now();
    } else if (tsRaw is num) {
      final v = tsRaw.toDouble();
      ts = DateTime.fromMillisecondsSinceEpoch(
        v > 1e12 ? v.toInt() : (v * 1000).toInt(),
        isUtc: true,
      ).toLocal();
    } else {
      ts = DateTime.now();
    }
    double n(dynamic x) => (x is num) ? x.toDouble() : double.tryParse('$x') ?? 0;
    return Telemetry(
      deviceId: '${j['device_id'] ?? j['id'] ?? 'PG-PROTO-001'}',
      ts: ts,
      va: n(j['Va'] ?? j['va']),
      vb: n(j['Vb'] ?? j['vb']),
      vc: n(j['Vc'] ?? j['vc']),
      ia: n(j['Ia'] ?? j['ia']),
      ib: n(j['Ib'] ?? j['ib']),
      ic: n(j['Ic'] ?? j['ic']),
      fault: '${j['fault'] ?? 'NONE'}'.toUpperCase(),
      relay: '${j['relay'] ?? 'CLOSED'}'.toUpperCase(),
    );
  }
}

class FaultEvent {
  FaultEvent({
    required this.id,
    required this.deviceId,
    required this.ts,
    required this.fault,
    required this.relay,
    required this.message,
    required this.va,
    required this.vb,
    required this.vc,
  });

  final int id;
  final String deviceId;
  final DateTime ts;
  final String fault;
  final String relay;
  final String message;
  final double va, vb, vc;

  factory FaultEvent.fromJson(Map<String, dynamic> j) {
    final tsRaw = j['ts'];
    DateTime ts;
    if (tsRaw is String) {
      ts = DateTime.tryParse(tsRaw)?.toLocal() ?? DateTime.now();
    } else {
      ts = DateTime.now();
    }
    double n(dynamic x) => (x is num) ? x.toDouble() : 0;
    return FaultEvent(
      id: j['id'] as int? ?? 0,
      deviceId: '${j['device_id'] ?? ''}',
      ts: ts,
      fault: '${j['fault'] ?? ''}',
      relay: '${j['relay'] ?? ''}',
      message: '${j['message'] ?? ''}',
      va: n(j['Va']),
      vb: n(j['Vb']),
      vc: n(j['Vc']),
    );
  }
}
