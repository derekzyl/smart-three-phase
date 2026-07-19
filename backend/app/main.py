"""FastAPI entrypoint for PG-PROTO-001 backend."""
from __future__ import annotations

import logging
from contextlib import asynccontextmanager

from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware

from app import mqtt_bridge as mqtt_mod
from app.api import router
from app.config import get_settings
from app.db import init_db
from app.mqtt_bridge import MqttBridge

logging.basicConfig(level=logging.INFO, format="%(asctime)s %(levelname)s [%(name)s] %(message)s")
logger = logging.getLogger("ai3phase")


@asynccontextmanager
async def lifespan(app: FastAPI):
    settings = get_settings()
    await init_db()
    mqtt_mod.bridge = MqttBridge(settings)
    import asyncio

    mqtt_mod.bridge.start(asyncio.get_running_loop())
    logger.info("Backend ready — device %s", settings.device_id)
    yield
    mqtt_mod.bridge.stop()


def create_app() -> FastAPI:
    settings = get_settings()
    app = FastAPI(
        title="AI3Phase Protection API",
        description="MQTT bridge + REST for Intelligent Three-Phase Load Protection (PG-PROTO-001)",
        version="0.1.0",
        lifespan=lifespan,
    )
    origins = [o.strip() for o in settings.cors_origins.split(",") if o.strip()]
    app.add_middleware(
        CORSMiddleware,
        allow_origins=origins if origins != ["*"] else ["*"],
        allow_credentials=True,
        allow_methods=["*"],
        allow_headers=["*"],
    )
    app.include_router(router, prefix="/api/v1")
    return app


app = create_app()


def run() -> None:
    import uvicorn

    settings = get_settings()
    uvicorn.run("app.main:app", host=settings.api_host, port=settings.api_port, reload=False)


if __name__ == "__main__":
    run()
