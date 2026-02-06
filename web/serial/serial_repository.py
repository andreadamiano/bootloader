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
    timeout=settings.serial.TIMEOUT,
)

class SerialRepository:
    def __init__(self):
        self.parser = SupParser()

    def read_sup_frame(self, timeout: float):
        try:
            while time.time() < timeout:
                if ser.in_waiting > 0: #get number of bytes in the input buffer
                    byte = ser.read(1)
                    frame = self.parser.handle_rx_byte(byte[0]) #byte[0] return the first byte of the bytes object
                    if frame:
                        return frame
            
            return None
        except Exception as e:
            logger.error(f"An error occurred while reading sup frame: {e}")
            return None

    def send_frame_and_wait_ack(self, frame: bytes, timeout: int = 10) -> bool:
        for _ in range(settings.serial.MAX_RETRIES):
            ser.write(frame) 
            sup_frame = self.read_sup_frame(time.time() + timeout) #wait for 10 seconds the ack message
            if sup_frame and sup_frame.frame_id == SupId.ACK and sup_frame.payload[0] == frame[1]:
                logger.info(f"Received ack message for id 0x{frame[1]:x}")
                return True
            elif sup_frame and sup_frame.frame_id == SupId.NACK:
                logger.error("Receive nack message")
                return False
            
            logger.error("Did not received the ack message before timeout")
        
        logger.error("Max tries reached before receiving the ack message")
        return False

    def send_firmware_update(self):
        #send update signal
        frame = create_frame(SupId.CMD_FW_UPDATE)
        self.send_frame_and_wait_ack(frame)

        #send firware size
        firware_size = 128
        frame = create_frame(SupId.DATA, firware_size.to_bytes(2, "little"))  # send firmware size as 2 bytes in little indian format 
        self.send_frame_and_wait_ack(frame)

        #read firmware and send it via usart
        with open("build/main_app.bin", "rb") as f:
            data = f.read()
            data_lenght = len(data)

            for i in range(0, data_lenght, MAX_PAYLOAD_SIZE):
                logger.debug(f"Frame: {data[i:min(i+MAX_PAYLOAD_SIZE, data_lenght)]}")
                frame = create_frame(SupId.DATA, data[i:min(i+MAX_PAYLOAD_SIZE, data_lenght)]) #send bytes in little endian format 
                self.send_frame_and_wait_ack(frame)

        self.listen()

    def send_sup_frame(self):
        pass

    def listen(self):
        if ser.is_open:
            print("seriall port is open, listening...")
        while True:
            data = ser.readline()
            if data:
                print(f"Received: {data}")

