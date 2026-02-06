from pydantic_settings import BaseSettings, SettingsConfigDict
from pydantic import Field

class SerialSettings(BaseSettings):
    SERIAL_PORT: str = Field(alias="SERIAL_PORT")
    BAUD_RATE: int = Field(alias="BAUD_RATE")
    MAX_RETRIES: int = Field(alias="MAX_RETRIES")
    TIMEOUT: int = Field(alias="TIMEOUT")

    model_config = SettingsConfigDict(
        env_file=".env", 
        extra="ignore"
    )

class Settings(BaseSettings):
    DEBUG: bool = Field(alias="DEBUG") 
    serial: SerialSettings = SerialSettings()

    model_config = SettingsConfigDict(
        env_file=".env",
        extra="ignore"
    )

settings = Settings()