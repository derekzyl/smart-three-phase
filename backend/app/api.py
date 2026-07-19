"""REST + WebSocket routes."""
from __future__ import annotations

import asyncio
import json
from datetime import datetime, timezone

from typing import Optional

from fastapi import APIRouter, Depends, HTTPException, Query, WebSocket, WebSocketDisconnect
from sqlalchemy import desc, select
from sqlalchemy.ext.asyncio import AsyncSession

from app.config import get_settings
from app.db import get_session
from app.models import EventRow, TelemetryRow
from app.mqtt_bridge import get_bridge
from app.schemas import EventOut, HealthOut, ResetCommand, TelemetryIn, TelemetryOut

router = APIRouter()


def _row_to_telemetry(row: TelemetryRow) -> TelemetryOut:
    return TelemetryOut(
        device_id=row.device_id,
        ts=row.ts,
        Va=row.va,
        Vb=row.vb,
        Vc=row.vc,
        Ia=row.ia,
        Ib=row.ib,
        Ic=row.ic,
        fault=row.fault,
        relay=row.relay,
    )


@router.get("/health", response_model=HealthOut)
async def health(session: AsyncSession = Depends(get_session)) -> HealthOut:
    settings = get_settings()
    bridge = get_bridge()
    result = await session.execute(select(TelemetryRow).order_by(desc(TelemetryRow.id)).limit(1))
    latest = result.scalar_one_or_none()
    return HealthOut(
        status="ok",
        device_id=settings.device_id,
        mqtt_connected=bridge.connected,
        latest_ts=latest.ts if latest else None,
    )


@router.get("/telemetry/latest", response_model=Optional[TelemetryOut])
async def telemetry_latest(session: AsyncSession = Depends(get_session)) -> Optional[TelemetryOut]:
    bridge = get_bridge()
    if bridge.latest:
        b = bridge.latest
        return TelemetryOut(
            device_id=b["device_id"],
            ts=b["ts"],
            Va=b["Va"],
            Vb=b["Vb"],
            Vc=b["Vc"],
            Ia=b["Ia"],
            Ib=b["Ib"],
            Ic=b["Ic"],
            fault=b["fault"],
            relay=b["relay"],
        )
    result = await session.execute(select(TelemetryRow).order_by(desc(TelemetryRow.id)).limit(1))
    row = result.scalar_one_or_none()
    return _row_to_telemetry(row) if row else None


@router.get("/telemetry/history", response_model=list[TelemetryOut])
async def telemetry_history(
    limit: int = Query(100, ge=1, le=1000),
    session: AsyncSession = Depends(get_session),
) -> list[TelemetryOut]:
    result = await session.execute(
        select(TelemetryRow).order_by(desc(TelemetryRow.id)).limit(limit)
    )
    rows = result.scalars().all()
    return [_row_to_telemetry(r) for r in reversed(rows)]


@router.get("/events", response_model=list[EventOut])
async def list_events(
    limit: int = Query(50, ge=1, le=500),
    fault: Optional[str] = None,
    session: AsyncSession = Depends(get_session),
) -> list[EventOut]:
    q = select(EventRow).order_by(desc(EventRow.id)).limit(limit)
    if fault:
        q = select(EventRow).where(EventRow.fault == fault.upper()).order_by(desc(EventRow.id)).limit(limit)
    result = await session.execute(q)
    rows = result.scalars().all()
    return [
        EventOut(
            id=r.id,
            device_id=r.device_id,
            ts=r.ts,
            fault=r.fault,
            relay=r.relay,
            Va=r.va,
            Vb=r.vb,
            Vc=r.vc,
            Ia=r.ia,
            Ib=r.ib,
            Ic=r.ic,
            message=r.message,
        )
        for r in rows
    ]


@router.post("/command/reset")
async def command_reset(body: ResetCommand) -> dict:
    settings = get_settings()
    if body.token != settings.api_reset_token:
        raise HTTPException(status_code=401, detail="Invalid token")
    bridge = get_bridge()
    payload = {"cmd": "RESET", "device_id": body.device_id, "ts": int(datetime.now(timezone.utc).timestamp())}
    bridge.publish_command(payload)
    return {"ok": True, "published": payload}


@router.post("/sim/telemetry", response_model=TelemetryOut, tags=["dev"])
async def sim_telemetry(body: TelemetryIn, session: AsyncSession = Depends(get_session)) -> TelemetryOut:
    """Dev helper: inject telemetry without MQTT (useful before hardware)."""
    bridge = get_bridge()
    raw = body.model_dump()
    await bridge.handle_payload("plant/sim/telemetry", raw)
    assert bridge.latest is not None
    b = bridge.latest
    return TelemetryOut(
        device_id=b["device_id"],
        ts=b["ts"],
        Va=b["Va"],
        Vb=b["Vb"],
        Vc=b["Vc"],
        Ia=b["Ia"],
        Ib=b["Ib"],
        Ic=b["Ic"],
        fault=b["fault"],
        relay=b["relay"],
    )


class ConnectionManager:
    def __init__(self) -> None:
        self.active: list[WebSocket] = []

    async def connect(self, ws: WebSocket) -> None:
        await ws.accept()
        self.active.append(ws)

    def disconnect(self, ws: WebSocket) -> None:
        if ws in self.active:
            self.active.remove(ws)

    async def broadcast(self, message: dict) -> None:
        dead: list[WebSocket] = []
        data = json.dumps(message, default=str)
        for ws in self.active:
            try:
                await ws.send_text(data)
            except Exception:
                dead.append(ws)
        for ws in dead:
            self.disconnect(ws)


ws_manager = ConnectionManager()


@router.websocket("/ws/live")
async def ws_live(websocket: WebSocket) -> None:
    await ws_manager.connect(websocket)
    bridge = get_bridge()
    queue: asyncio.Queue = asyncio.Queue()

    def on_msg(payload: dict) -> None:
        try:
            queue.put_nowait(payload)
        except Exception:
            pass

    bridge.add_listener(on_msg)
    try:
        if bridge.latest:
            await websocket.send_text(json.dumps(bridge.latest, default=str))
        while True:
            try:
                item = await asyncio.wait_for(queue.get(), timeout=25.0)
                await websocket.send_text(json.dumps(item, default=str))
            except asyncio.TimeoutError:
                await websocket.send_text(json.dumps({"type": "ping"}))
    except WebSocketDisconnect:
        ws_manager.disconnect(websocket)
