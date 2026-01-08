from pydantic_settings import BaseSettings
from pydantic import Field

class SerialSettings(BaseSettings):
    SERIAL_PORT: str = Field(alias="SERIAL_PORT")
    BAUS_RATE: int = Field(alias="BAUS_RATE")

class Settings(BaseSettings):
    serial = SerialSettings()



settings = Settings()