"""SQLAlchemy models."""
from datetime import datetime, timezone

from sqlalchemy import DateTime, Float, Integer, String, Text
from sqlalchemy.orm import DeclarativeBase, Mapped, mapped_column


def utcnow() -> datetime:
    return datetime.now(timezone.utc)


class Base(DeclarativeBase):
    pass


class TelemetryRow(Base):
    __tablename__ = "telemetry"

    id: Mapped[int] = mapped_column(Integer, primary_key=True, autoincrement=True)
    device_id: Mapped[str] = mapped_column(String(64), index=True)
    ts: Mapped[datetime] = mapped_column(DateTime(timezone=True), index=True, default=utcnow)
    va: Mapped[float] = mapped_column(Float, default=0.0)
    vb: Mapped[float] = mapped_column(Float, default=0.0)
    vc: Mapped[float] = mapped_column(Float, default=0.0)
    ia: Mapped[float] = mapped_column(Float, default=0.0)
    ib: Mapped[float] = mapped_column(Float, default=0.0)
    ic: Mapped[float] = mapped_column(Float, default=0.0)
    fault: Mapped[str] = mapped_column(String(16), default="NONE")
    relay: Mapped[str] = mapped_column(String(16), default="CLOSED")
    raw_json: Mapped[str] = mapped_column(Text, default="{}")


class EventRow(Base):
    __tablename__ = "events"

    id: Mapped[int] = mapped_column(Integer, primary_key=True, autoincrement=True)
    device_id: Mapped[str] = mapped_column(String(64), index=True)
    ts: Mapped[datetime] = mapped_column(DateTime(timezone=True), index=True, default=utcnow)
    fault: Mapped[str] = mapped_column(String(16), index=True)
    relay: Mapped[str] = mapped_column(String(16), default="OPEN")
    va: Mapped[float] = mapped_column(Float, default=0.0)
    vb: Mapped[float] = mapped_column(Float, default=0.0)
    vc: Mapped[float] = mapped_column(Float, default=0.0)
    ia: Mapped[float] = mapped_column(Float, default=0.0)
    ib: Mapped[float] = mapped_column(Float, default=0.0)
    ic: Mapped[float] = mapped_column(Float, default=0.0)
    message: Mapped[str] = mapped_column(Text, default="")
    raw_json: Mapped[str] = mapped_column(Text, default="{}")
