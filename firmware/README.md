# PG-PROTO-001 ESP32 Firmware

## Configure

Edit `include/secrets.h` (Wi‑Fi, MQTT broker IP, SMS number).

Optional build flags in `platformio.ini`:

```ini
build_flags =
  -DDEMO_MODE=1
  -DENABLE_GSM=0
```

`DEMO_MODE=1` synthesises healthy RMS (and occasional phase-loss) without sensors — good for bench bring-up of MQTT/OLED/relay paths.

## Build / upload

```bash
cd firmware
pio run
pio run -t upload
pio device monitor
```

## MQTT topics

- `plant/PG-PROTO-001/telemetry` — 1 Hz JSON
- `plant/PG-PROTO-001/faults` — fault edge events
- `plant/PG-PROTO-001/command` — `{"cmd":"RESET"}` from backend
