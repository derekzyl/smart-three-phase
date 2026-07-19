"""Application settings loaded from environment / .env."""
from functools import lru_cache

from pydantic_settings import BaseSettings, SettingsConfigDict


class Settings(BaseSettings):
    model_config = SettingsConfigDict(env_file=".env", env_file_encoding="utf-8", extra="ignore")

    device_id: str = "PG-PROTO-001"
    api_host: str = "0.0.0.0"
    api_port: int = 8000
    api_reset_token: str = "change-me"

    mqtt_host: str = "127.0.0.1"
    mqtt_port: int = 1883
    mqtt_username: str = ""
    mqtt_password: str = ""
    mqtt_topic_telemetry: str = "plant/PG-PROTO-001/telemetry"
    mqtt_topic_faults: str = "plant/PG-PROTO-001/faults"
    mqtt_topic_command: str = "plant/PG-PROTO-001/command"

    database_url: str = "sqlite+aiosqlite:///./data/ai3phase.db"
    cors_origins: str = "*"


@lru_cache
def get_settings() -> Settings:
    return Settings()
