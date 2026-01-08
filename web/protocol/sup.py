
# SUP Protocol Constants
SUP_SOF = 0xA1
SUP_EOF = 0xE9
SUP_MAX_PAYLOAD_SIZE = 128

# SUP Frame IDs
SUP_ID_ACK = 0x01  # Acknowledgment for received frame
SUP_ID_NACK = 0x02  # Negative Acknowledgment for received frame
SUP_ID_DATA = 0x03  # Binary data chunk transfer
SUP_ID_CMD_FW_UPDATE = 0x11  # Firmware Update Command


# SUP Protocol Functions
def calculate_checksum(payload_size, id, payload):
    """Calculates the checksum for a SUP frame."""
    checksum = payload_size + id
    for byte in payload:
        checksum += byte
    return checksum & 0xFF  # Ensure checksum is a single byte


def create_sup_frame(id, payload=b""):
    """Creates a complete SUP frame as a bytearray."""
    payload_size = len(payload)
    if payload_size > SUP_MAX_PAYLOAD_SIZE:
        raise ValueError(
            f"Payload size ({payload_size}) exceeds max limit ({SUP_MAX_PAYLOAD_SIZE})"
        )

    checksum = calculate_checksum(payload_size, id, payload)

    frame = bytearray()
    frame.append(SUP_SOF)
    frame.append(id)
    frame.append(payload_size)
    frame.extend(payload)
    frame.append(checksum)
    frame.append(SUP_EOF)

    return frame


def parse_sup_frames(buffer: bytearray):
    """Parse a byte buffer and return a list of full SUP frames (each as bytearray).

    A SUP frame has the structure:
      SOF(1) ID(1) SIZE(1) PAYLOAD(SIZE) CHECKSUM(1) EOF(1)

    This function extracts complete frames and skips incomplete or corrupted segments.
    """
    frames = []
    i = 0
    buf_len = len(buffer)
    while i < buf_len:
        # find next SOF
        if buffer[i] != SUP_SOF:
            i += 1
            continue

        # need at least SOF + ID + SIZE + CHECKSUM + EOF => 5 bytes (with 0 payload)
        if i + 5 > buf_len:
            # incomplete header/short tail -- wait for more bytes
            break

        # read size
        payload_size = buffer[i + 2]
        total_len = payload_size + 5  # SOF, ID, SIZE, PAYLOAD, CHK, EOF => payload+5

        # check if full frame is present
        if i + total_len > buf_len:
            # incomplete frame
            break

        # check EOF marker
        if buffer[i + total_len - 1] != SUP_EOF:
            # malformed frame: skip this SOF and continue searching
            i += 1
            continue

        # extract full frame
        frame = bytearray(buffer[i : i + total_len])
        frames.append(frame)

        # advance index past this frame
        i += total_len

    return frames
