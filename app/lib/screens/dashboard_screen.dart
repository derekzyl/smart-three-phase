import 'package:flutter/material.dart';
import 'package:intl/intl.dart';

import '../models/telemetry.dart';
import '../services/api_service.dart';

class DashboardScreen extends StatefulWidget {
  const DashboardScreen({super.key, required this.api});

  final ApiService api;

  @override
  State<DashboardScreen> createState() => _DashboardScreenState();
}

class _DashboardScreenState extends State<DashboardScreen> {
  Telemetry? _t;
  List<FaultEvent> _events = [];
  String? _error;
  bool _loading = true;
  final _tokenCtrl = TextEditingController(text: 'change-me');

  @override
  void initState() {
    super.initState();
    _bootstrap();
  }

  Future<void> _bootstrap() async {
    setState(() {
      _loading = true;
      _error = null;
    });
    try {
      final latest = await widget.api.latest();
      final events = await widget.api.events();
      if (!mounted) return;
      setState(() {
        _t = latest;
        _events = events;
        _loading = false;
      });
      widget.api.connectLive(
        (t) {
          if (!mounted) return;
          setState(() => _t = t);
          if (t.fault != 'NONE') _refreshEvents();
        },
        onError: (e) {
          if (!mounted) return;
          setState(() => _error = 'Live link: $e');
        },
      );
    } catch (e) {
      if (!mounted) return;
      setState(() {
        _error = '$e';
        _loading = false;
      });
    }
  }

  Future<void> _refreshEvents() async {
    final events = await widget.api.events();
    if (mounted) setState(() => _events = events);
  }

  Future<void> _reset() async {
    final ok = await widget.api.reset(token: _tokenCtrl.text.trim());
    if (!mounted) return;
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(content: Text(ok ? 'RESET published to device' : 'Reset failed')),
    );
  }

  Future<void> _simHealthy() async {
    await widget.api.injectSim();
    await _refreshEvents();
  }

  Future<void> _simPhaseLoss() async {
    await widget.api.injectSim(va: 12, fault: 'PL', relay: 'OPEN');
    await _refreshEvents();
  }

  @override
  void dispose() {
    widget.api.dispose();
    _tokenCtrl.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final t = _t;
    final healthy = t?.isHealthy ?? false;
    final statusColor = t == null
        ? Colors.grey
        : healthy
            ? const Color(0xFF1B7F4E)
            : const Color(0xFFB3261E);

    return Scaffold(
      backgroundColor: const Color(0xFFF3F1EC),
      appBar: AppBar(
        title: const Text('PG-PROTO-001 Monitor'),
        backgroundColor: const Color(0xFF1C2B3A),
        foregroundColor: Colors.white,
        actions: [
          IconButton(onPressed: _bootstrap, icon: const Icon(Icons.refresh)),
        ],
      ),
      body: _loading
          ? const Center(child: CircularProgressIndicator())
          : RefreshIndicator(
              onRefresh: _bootstrap,
              child: ListView(
                padding: const EdgeInsets.all(16),
                children: [
                  if (_error != null)
                    Card(
                      color: const Color(0xFFFFE8E6),
                      child: Padding(
                        padding: const EdgeInsets.all(12),
                        child: Text(_error!, style: const TextStyle(color: Color(0xFF8B1A1A))),
                      ),
                    ),
                  Container(
                    padding: const EdgeInsets.all(20),
                    decoration: BoxDecoration(
                      color: statusColor,
                      borderRadius: BorderRadius.circular(4),
                    ),
                    child: Column(
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: [
                        Text(
                          t == null ? 'NO DATA' : (healthy ? 'HEALTHY' : 'TRIPPED · ${t.fault}'),
                          style: const TextStyle(
                            color: Colors.white,
                            fontSize: 22,
                            fontWeight: FontWeight.w700,
                            letterSpacing: 0.5,
                          ),
                        ),
                        const SizedBox(height: 6),
                        Text(
                          t == null
                              ? 'Waiting for MQTT / sim telemetry…'
                              : 'Relay ${t.relay} · ${DateFormat('HH:mm:ss').format(t.ts)}',
                          style: const TextStyle(color: Colors.white70),
                        ),
                      ],
                    ),
                  ),
                  const SizedBox(height: 16),
                  const Text('Phase voltages (Vrms)', style: TextStyle(fontWeight: FontWeight.w600)),
                  const SizedBox(height: 8),
                  Row(
                    children: [
                      _metric('Va', t?.va),
                      _metric('Vb', t?.vb),
                      _metric('Vc', t?.vc),
                    ],
                  ),
                  const SizedBox(height: 16),
                  const Text('Phase currents (A)', style: TextStyle(fontWeight: FontWeight.w600)),
                  const SizedBox(height: 8),
                  Row(
                    children: [
                      _metric('Ia', t?.ia, digits: 2),
                      _metric('Ib', t?.ib, digits: 2),
                      _metric('Ic', t?.ic, digits: 2),
                    ],
                  ),
                  const SizedBox(height: 20),
                  TextField(
                    controller: _tokenCtrl,
                    decoration: const InputDecoration(
                      labelText: 'Reset API token',
                      border: OutlineInputBorder(),
                      isDense: true,
                    ),
                  ),
                  const SizedBox(height: 8),
                  Wrap(
                    spacing: 8,
                    runSpacing: 8,
                    children: [
                      FilledButton(
                        onPressed: _reset,
                        style: FilledButton.styleFrom(backgroundColor: const Color(0xFF1C2B3A)),
                        child: const Text('Send RESET'),
                      ),
                      OutlinedButton(onPressed: _simHealthy, child: const Text('Sim healthy')),
                      OutlinedButton(onPressed: _simPhaseLoss, child: const Text('Sim PL fault')),
                    ],
                  ),
                  const SizedBox(height: 24),
                  Row(
                    children: [
                      const Text('Fault history', style: TextStyle(fontWeight: FontWeight.w600, fontSize: 16)),
                      const Spacer(),
                      TextButton(onPressed: _refreshEvents, child: const Text('Refresh')),
                    ],
                  ),
                  if (_events.isEmpty)
                    const Padding(
                      padding: EdgeInsets.symmetric(vertical: 24),
                      child: Text('No events yet'),
                    )
                  else
                    ..._events.map(
                      (e) => ListTile(
                        contentPadding: EdgeInsets.zero,
                        leading: CircleAvatar(
                          backgroundColor: const Color(0xFFB3261E),
                          child: Text(e.fault, style: const TextStyle(color: Colors.white, fontSize: 11)),
                        ),
                        title: Text(e.message.isEmpty ? e.fault : e.message),
                        subtitle: Text(
                          '${DateFormat('yyyy-MM-dd HH:mm:ss').format(e.ts)} · '
                          'Va ${e.va.toStringAsFixed(0)} Vb ${e.vb.toStringAsFixed(0)} Vc ${e.vc.toStringAsFixed(0)}',
                        ),
                      ),
                    ),
                ],
              ),
            ),
    );
  }

  Widget _metric(String label, double? v, {int digits = 1}) {
    return Expanded(
      child: Container(
        margin: const EdgeInsets.only(right: 8),
        padding: const EdgeInsets.symmetric(vertical: 14, horizontal: 10),
        decoration: BoxDecoration(
          color: Colors.white,
          border: Border.all(color: const Color(0xFFD9D3C8)),
        ),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text(label, style: const TextStyle(color: Color(0xFF5C584F), fontSize: 12)),
            const SizedBox(height: 4),
            Text(
              v == null ? '—' : v.toStringAsFixed(digits),
              style: const TextStyle(fontSize: 20, fontWeight: FontWeight.w600),
            ),
          ],
        ),
      ),
    );
  }
}
