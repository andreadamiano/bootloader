from protocol.sup import *
import serial
from web.core.settings import settings

ser = serial.Serial(
    port=settings.serial.SERIAL_PORT,
    baudrate=settings.serial.BAUS_RATE,
    parity=serial.PARITY_ODD,
    stopbits=serial.STOPBITS_TWO,
    bytesize=serial.SEVENBITS
)



def send_hello():
    ser.write(b"hello")

def send_firmware_update():
    pass 

def send_sup_frame():
    pass

