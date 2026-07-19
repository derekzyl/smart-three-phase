# AI3Phase Backend (FastAPI + uv)

MQTT → SQLite bridge and REST/WebSocket API for the PG-PROTO-001 dashboard.

## Quick start

```bash
cd backend
cp .env.example .env
uv sync
uv run uvicorn app.main:app --host 0.0.0.0 --port 8000 --reload
```

Optional MQTT broker (Mosquitto):

```bash
sudo apt install mosquitto mosquitto-clients
# or: docker run -p 1883:1883 eclipse-mosquitto:2
```

## API

| Method | Path | Purpose |
|--------|------|---------|
| GET | `/api/v1/health` | Service + MQTT status |
| GET | `/api/v1/telemetry/latest` | Latest RMS / fault / relay |
| GET | `/api/v1/telemetry/history` | Recent samples |
| GET | `/api/v1/events` | Fault event log |
| POST | `/api/v1/command/reset` | Publish RESET to device MQTT |
| POST | `/api/v1/sim/telemetry` | Inject sample without hardware |
| WS | `/api/v1/ws/live` | Live JSON stream |

Docs: http://127.0.0.1:8000/docs
