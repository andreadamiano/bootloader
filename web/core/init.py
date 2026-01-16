import logging 
import colorlog

LOGGER_FORMAT = "[%(asctime)-15s] %(log_color)s%(levelname)s: [Process-%(process)d] (%(threadName)s.%(module)s.%(funcName)s - %(message)s"
LOGGER_HANDLER = colorlog.StreamHandler()
LOGGER_HANDLER.setFormatter(colorlog.ColoredFormatter(LOGGER_FORMAT))

def setup_loggings(debug: bool = False):
    logging.basicConfig(
        format=LOGGER_FORMAT, 
        level=logging.DEBUG if debug else logging.INFO, 
        handlers=[LOGGER_HANDLER]
    )