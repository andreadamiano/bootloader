from pydantic_settings import BaseSettings, SettingsConfigDict
from pydantic import Field

class SerialSettings(BaseSettings):
    SERIAL_PORT: str = Field(alias="SERIAL_PORT")
    BAUD_RATE: int = Field(alias="BAUD_RATE")
    MAX_RETRIES: int = Field(alias="MAX_RETRIES")

    model_config = SettingsConfigDict(
        env_file=".env"
    )

class Settings(BaseSettings):
    serial: SerialSettings = SerialSettings()



settings = Settings()