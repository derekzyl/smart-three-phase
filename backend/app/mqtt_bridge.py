"""MQTT subscriber → persistence + live fan-out."""
from __future__ import annotations

import asyncio
import json
import logging
from datetime import datetime, timezone
from typing import Any, Callable, Optional

import paho.mqtt.client as mqtt

from app.config import Settings
from app.db import SessionLocal
from app.models import EventRow, TelemetryRow

logger = logging.getLogger(__name__)


class MqttBridge:
    def __init__(self, settings: Settings) -> None:
        self.settings = settings
        self.connected = False
        self.latest: Optional[dict[str, Any]] = None
        self._last_fault: str = "NONE"
        self._loop: Optional[asyncio.AbstractEventLoop] = None
        self._listeners: list[Callable[[dict[str, Any]], None]] = []
        self._client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id="ai3phase-backend")
        if settings.mqtt_username:
            self._client.username_pw_set(settings.mqtt_username, settings.mqtt_password or None)
        self._client.on_connect = self._on_connect
        self._client.on_disconnect = self._on_disconnect
        self._client.on_message = self._on_message

    def add_listener(self, cb: Callable[[dict[str, Any]], None]) -> None:
        self._listeners.append(cb)

    def start(self, loop: asyncio.AbstractEventLoop) -> None:
        self._loop = loop
        try:
            self._client.connect_async(self.settings.mqtt_host, self.settings.mqtt_port, keepalive=30)
            self._client.loop_start()
            logger.info("MQTT connecting to %s:%s", self.settings.mqtt_host, self.settings.mqtt_port)
        except Exception:
            logger.exception("MQTT connect failed — API still runs; use /sim/telemetry")

    def stop(self) -> None:
        try:
            self._client.loop_stop()
            self._client.disconnect()
        except Exception:
            pass
        self.connected = False

    def publish_command(self, payload: dict[str, Any]) -> None:
        data = json.dumps(payload)
        self._client.publish(self.settings.mqtt_topic_command, data, qos=1)

    def _on_connect(self, client, userdata, flags, reason_code, properties=None) -> None:
        self.connected = reason_code == 0 or str(reason_code) in ("Success", "0")
        if self.connected:
            client.subscribe(self.settings.mqtt_topic_telemetry, qos=0)
            client.subscribe(self.settings.mqtt_topic_faults, qos=1)
            logger.info("MQTT connected; subscribed to telemetry/faults")
        else:
            logger.warning("MQTT connect reason: %s", reason_code)

    def _on_disconnect(self, client, userdata, flags, reason_code, properties=None) -> None:
        self.connected = False
        logger.warning("MQTT disconnected: %s", reason_code)

    def _on_message(self, client, userdata, msg) -> None:
        try:
            payload = json.loads(msg.payload.decode("utf-8"))
        except Exception:
            logger.warning("Invalid JSON on %s", msg.topic)
            return
        if self._loop is None:
            return
        asyncio.run_coroutine_threadsafe(self.handle_payload(msg.topic, payload), self._loop)

    async def handle_payload(self, topic: str, payload: dict[str, Any]) -> None:
        """Public so /sim/telemetry can reuse the same pipeline."""
        normalized = self._normalize(payload)
        self.latest = normalized
        for cb in list(self._listeners):
            try:
                cb(normalized)
            except Exception:
                logger.exception("listener error")

        fault = normalized["fault"]
        is_fault_topic = "fault" in topic
        edge = fault != "NONE" and fault != self._last_fault
        if fault == "NONE":
            self._last_fault = "NONE"
        elif edge or is_fault_topic:
            self._last_fault = fault

        async with SessionLocal() as session:
            session.add(
                TelemetryRow(
                    device_id=normalized["device_id"],
                    ts=normalized["ts"],
                    va=normalized["Va"],
                    vb=normalized["Vb"],
                    vc=normalized["Vc"],
                    ia=normalized["Ia"],
                    ib=normalized["Ib"],
                    ic=normalized["Ic"],
                    fault=normalized["fault"],
                    relay=normalized["relay"],
                    raw_json=json.dumps(payload),
                )
            )
            if is_fault_topic or edge:
                session.add(
                    EventRow(
                        device_id=normalized["device_id"],
                        ts=normalized["ts"],
                        fault=fault,
                        relay=normalized["relay"],
                        va=normalized["Va"],
                        vb=normalized["Vb"],
                        vc=normalized["Vc"],
                        ia=normalized["Ia"],
                        ib=normalized["Ib"],
                        ic=normalized["Ic"],
                        message=str(payload.get("message", f"Fault {fault}")),
                        raw_json=json.dumps(payload),
                    )
                )
            await session.commit()

    def _normalize(self, payload: dict[str, Any]) -> dict[str, Any]:
        ts_raw = payload.get("ts")
        if isinstance(ts_raw, (int, float)):
            ts = datetime.fromtimestamp(ts_raw / 1000.0 if ts_raw > 1e12 else ts_raw, tz=timezone.utc)
        else:
            ts = datetime.now(timezone.utc)
        return {
            "device_id": str(payload.get("id") or payload.get("device_id") or self.settings.device_id),
            "ts": ts,
            "Va": float(payload.get("Va", payload.get("va", 0.0))),
            "Vb": float(payload.get("Vb", payload.get("vb", 0.0))),
            "Vc": float(payload.get("Vc", payload.get("vc", 0.0))),
            "Ia": float(payload.get("Ia", payload.get("ia", 0.0))),
            "Ib": float(payload.get("Ib", payload.get("ib", 0.0))),
            "Ic": float(payload.get("Ic", payload.get("ic", 0.0))),
            "fault": str(payload.get("fault", "NONE")).upper(),
            "relay": str(payload.get("relay", "CLOSED")).upper(),
        }


bridge: Optional[MqttBridge] = None


def get_bridge() -> MqttBridge:
    if bridge is None:
        raise RuntimeError("MQTT bridge not initialised")
    return bridge
