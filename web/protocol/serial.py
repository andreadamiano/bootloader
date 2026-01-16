from web.protocol.sup import *
import serial
from web.core.settings import settings
from enum import Enum


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
    frame = create_sup_frame(SUP_ID_CMD_FW_UPDATE)
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

