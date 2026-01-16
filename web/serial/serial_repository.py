from web.serial.sup import *
import serial
from web.core.settings import settings
import time
from web.core.init import setup_loggings
import logging

setup_loggings()
logger = logging.getLogger(__name__)

ser = serial.Serial(
    port=settings.serial.SERIAL_PORT,
    baudrate=settings.serial.BAUD_RATE,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    # timeout=0.1,
    timeout=None
)

class SerialRepository:
    def __init__(self):
        self.parser = SupParser()

    def read_sup_frame(self, timeout: float):
        try:
            while self.parser._state != SupParsingResult.SUCCESS and time.time() < timeout:
                byte = ser.read(1)
                self.parser.handle_rx_byte(byte)
            
            return False
        except Exception as e:
            logger.error(f"An error occurred while sending sup frame: {e}")

    def send_frame_and_wait_ack(self, frame: bytes) -> bool:
        for tries in range(settings.serial.MAX_RETRIES):
            ser.write(frame) 
            self.read_sup_frame()


    def send_firmware_update():
        # Use the defined SUP firmware update command ID to match the device protocol
        frame = create_frame(SupId.CMD_FW_UPDATE)
        ser.write(frame)

        

        frame = create_frame(SupId.DATA, b"128")  #firware size
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

