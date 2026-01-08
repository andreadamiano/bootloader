from web.protocol.sup import *
import serial
from web.core.settings import settings
from enum import Enum

class BootPacket(Enum):
    BOOT_REQUEST_UPDATE = 0
    BOOT_DATA = 1
    BOOT_MAX = 2


ser = serial.Serial(
    port=settings.serial.SERIAL_PORT,
    baudrate=settings.serial.BAUS_RATE,
    parity=serial.PARITY_ODD,
    stopbits=serial.STOPBITS_TWO,
    bytesize=serial.SEVENBITS
)

def send_firmware_update():
    frame = create_sup_frame(BootPacket.BOOT_REQUEST_UPDATE.value)
    ser.write(frame)

def send_sup_frame():
    pass

