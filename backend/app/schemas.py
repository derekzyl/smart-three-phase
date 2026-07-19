"""Pydantic API schemas."""
from datetime import datetime
from typing import Literal, Optional

from pydantic import BaseModel, Field


FaultCode = Literal["NONE", "OV", "UV", "OC", "PL", "IMB"]
RelayState = Literal["CLOSED", "OPEN"]


class TelemetryIn(BaseModel):
    schema_ver: int = 1
    id: str = "PG-PROTO-001"
    ts: Optional[int] = None
    Va: float = 0.0
    Vb: float = 0.0
    Vc: float = 0.0
    Ia: float = 0.0
    Ib: float = 0.0
    Ic: float = 0.0
    fault: str = "NONE"
    relay: str = "CLOSED"


class TelemetryOut(BaseModel):
    device_id: str
    ts: datetime
    Va: float
    Vb: float
    Vc: float
    Ia: float
    Ib: float
    Ic: float
    fault: str
    relay: str


class EventOut(BaseModel):
    id: int
    device_id: str
    ts: datetime
    fault: str
    relay: str
    Va: float
    Vb: float
    Vc: float
    Ia: float
    Ib: float
    Ic: float
    message: str


class ResetCommand(BaseModel):
    token: str = Field(..., description="API reset token")
    device_id: str = "PG-PROTO-001"


class HealthOut(BaseModel):
    status: str
    device_id: str
    mqtt_connected: bool
    latest_ts: Optional[datetime] = None
