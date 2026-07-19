# AI3Phase Flutter Dashboard

Talks to the FastAPI backend (`/api/v1/...`).

## Run

Start backend first, then:

```bash
cd app
flutter pub get
flutter run -d chrome --dart-define=API_BASE=http://127.0.0.1:8000
# or Linux desktop:
flutter run -d linux --dart-define=API_BASE=http://127.0.0.1:8000
```

Use **Sim healthy / Sim PL fault** buttons to exercise the UI without MQTT/hardware.
