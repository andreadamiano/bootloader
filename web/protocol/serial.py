from web.protocol.sup import *
import serial
from web.core.settings import settings
from enum import Enum

class BootPacket(Enum):
    BOOT_REQUEST_UPDATE = 0x00
    BOOT_DATA = 0x01
    BOOT_MAX = 0x02


ser = serial.Serial(
    port=settings.serial.SERIAL_PORT,
    baudrate=settings.serial.BAUD_RATE,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    # timeout=0.1,
    timeout=None
)

def send_firmware_update():
    # Use the defined SUP firmware update command ID to match the device protocol
    frame = create_sup_frame(BootPacket.BOOT_REQUEST_UPDATE.value)
    ser.write(frame)

def send_sup_frame():
    pass

def listen():
    if ser.is_open:
        print("seriall port is open, listening...")
    while True:
        data = ser.readline()
        if data:
            print(f"Received: {data}")

