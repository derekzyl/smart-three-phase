import 'package:flutter/material.dart';

import 'screens/dashboard_screen.dart';
import 'services/api_service.dart';

void main() {
  WidgetsFlutterBinding.ensureInitialized();
  runApp(const Ai3PhaseApp());
}

class Ai3PhaseApp extends StatelessWidget {
  const Ai3PhaseApp({super.key});

  @override
  Widget build(BuildContext context) {
    final api = ApiService();
    return MaterialApp(
      title: 'AI3Phase',
      debugShowCheckedModeBanner: false,
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(
          seedColor: const Color(0xFF1C2B3A),
          brightness: Brightness.light,
        ),
        useMaterial3: true,
        fontFamily: 'Roboto',
      ),
      home: DashboardScreen(api: api),
    );
  }
}
