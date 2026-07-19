import 'package:flutter_test/flutter_test.dart';
import 'package:ai3phase_app/main.dart';

void main() {
  testWidgets('Dashboard loads', (WidgetTester tester) async {
    await tester.pumpWidget(const Ai3PhaseApp());
    await tester.pump();
    expect(find.textContaining('PG-PROTO-001'), findsOneWidget);
  });
}
