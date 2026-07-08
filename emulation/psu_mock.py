import socket
import struct
import random

PSU_ADDR_7BIT = 0x58
WRITE_ADDR = (PSU_ADDR_7BIT << 1) | 0  # 0xB0
READ_ADDR = (PSU_ADDR_7BIT << 1) | 1   # 0xB1
CMD_READ_EIN = 0x86


class PsuState:
    def __init__(self):
        self.p_accum = 0
        self.n_samples = 0
        self.target_power = 10.0

    def update(self):
        new_samples = random.randint(1, 10)
        sample_value = int(self.target_power * new_samples)
        self.p_accum += sample_value
        self.n_samples += new_samples
        if self.p_accum > 0x7FFF:
            self.p_accum -= 0x8000


def calc_crc8(data_bytes):
    crc = 0
    for byte in data_bytes:
        crc ^= byte
        for _ in range(8):
            if crc & 0x80:
                crc = ((crc << 1) ^ 0x07) & 0xFF
            else:
                crc = (crc << 1) & 0xFF
    return crc


def handle_transaction(conn, state):
    try:
        header = conn.recv(2)
        if len(header) < 2:
            return False

        write_addr, cmd = header[0], header[1]
        if write_addr != WRITE_ADDR or cmd != CMD_READ_EIN:
            return False

        read_phase = conn.recv(1)
        if len(read_phase) < 1 or read_phase[0] != READ_ADDR:
            return False

        state.update()

        p_accum_bytes = struct.pack('<H', state.p_accum & 0xFFFF)
        n_samples_bytes = struct.pack('<I', state.n_samples & 0xFFFFFF)[:3]

        payload = p_accum_bytes + n_samples_bytes
        byte_count = len(payload)

        pec_input = bytes([WRITE_ADDR, CMD_READ_EIN, READ_ADDR, byte_count]) + payload
        pec = calc_crc8(pec_input)

        response = bytes([byte_count]) + payload + bytes([pec])
        conn.sendall(response)
        return True

    except (ConnectionResetError, BrokenPipeError, OSError):
        return False
    except Exception:
        return False


def run_emulator(host='127.0.0.1', port=9999):
    state = PsuState()
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((host, port))
        s.listen(1)

        while True:
            conn, _ = s.accept()
            try:
                while True:
                    if not handle_transaction(conn, state):
                        break
            finally:
                conn.close()


if __name__ == '__main__':
    run_emulator()