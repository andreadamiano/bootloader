from pydantic_settings import BaseSettings, SettingsConfigDict
from pydantic import Field

class SerialSettings(BaseSettings):
    SERIAL_PORT: str = Field(alias="SERIAL_PORT")
    BAUS_RATE: int = Field(alias="BAUS_RATE")

    model_config = SettingsConfigDict(
        env_file=".env"
    )

class Settings(BaseSettings):
    serial: SerialSettings = SerialSettings()



settings = Settings()