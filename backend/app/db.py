"""Async SQLAlchemy engine and session helpers."""
from collections.abc import AsyncGenerator
from pathlib import Path

from sqlalchemy.ext.asyncio import AsyncSession, async_sessionmaker, create_async_engine

from app.config import get_settings
from app.models import Base

_settings = get_settings()

# Ensure SQLite directory exists
if _settings.database_url.startswith("sqlite"):
    # sqlite+aiosqlite:///./data/ai3phase.db
    raw = _settings.database_url.split("///")[-1]
    Path(raw).parent.mkdir(parents=True, exist_ok=True)

engine = create_async_engine(_settings.database_url, echo=False)
SessionLocal = async_sessionmaker(engine, expire_on_commit=False, class_=AsyncSession)


async def init_db() -> None:
    async with engine.begin() as conn:
        await conn.run_sync(Base.metadata.create_all)


async def get_session() -> AsyncGenerator[AsyncSession, None]:
    async with SessionLocal() as session:
        yield session
