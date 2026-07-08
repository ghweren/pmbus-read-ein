import socket
import struct
import time
import sys
import EasyMCP2221

PSU_ADDR_7BIT = 0x58
WRITE_ADDR = (PSU_ADDR_7BIT << 1) | 0
READ_ADDR = (PSU_ADDR_7BIT << 1) | 1
CMD_READ_EIN = 0x86
POLL_INTERVAL_MS = 100
HOST = '127.0.0.1'
PORT = 9999

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

def form_read_block_request(cmd):
    return bytes([WRITE_ADDR, cmd, READ_ADDR])

def verify_and_parse_response(response_bytes):
    if len(response_bytes) < 7:
        raise ValueError(f"Некорректная длина ответа: {len(response_bytes)} байт")
    
    byte_count = response_bytes[0]
    pec_received = response_bytes[-1]
    data_payload = response_bytes[1:-1]
    
    if byte_count != len(data_payload):
        raise ValueError(f"Несоответствие Byte Count: ожидалось {byte_count}, получено {len(data_payload)}")
    
    pec_input = bytes([WRITE_ADDR, CMD_READ_EIN, READ_ADDR, byte_count]) + data_payload
    pec_calculated = calc_crc8(pec_input)
    
    if pec_received != pec_calculated:
        raise ValueError(f"Ошибка PEC! Получено: 0x{pec_received:02X}, Вычислено: 0x{pec_calculated:02X}")

    p_accum = struct.unpack_from('<H', data_payload, 0)[0]
    
    n_bytes = data_payload[2:5]
    if len(n_bytes) != 3:
        raise ValueError(f"Ожидалось 3 байта для Sample Count, получено {len(n_bytes)}")
        
    n_samples = n_bytes[0] | (n_bytes[1] << 8) | (n_bytes[2] << 16)
    
    if n_samples == 0:
        raise ZeroDivisionError("Sample count равен нулю")
        
    power_avg = p_accum / n_samples
    
    return {
        'p_accum': p_accum,
        'n_samples': n_samples,
        'power_watts': power_avg,
        'pec_ok': True
    }

def run_host_poller():
    print("[HOST] Инициализация MCP2221...")
    #print(f"[HOST] Подключение к PSU Mock на {HOST}:{PORT}...")
    print(f"[HOST] Адрес устройства: 0x{PSU_ADDR_7BIT<<1:02X}h (7-bit: 0x{PSU_ADDR_7BIT:02X})")
    print(f"[HOST] Команда: READ_EIN (0x{CMD_READ_EIN:02X})")
    print(f"[HOST] Интервал опроса: {POLL_INTERVAL_MS} мс\n")
    
    try:
        mcp = EasyMCP2221.Device()
        #with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            #s.connect((HOST, PORT))
            #s.settimeout(1.0)
            
        while True:
            start_time = time.time()

            #request = form_read_block_request(CMD_READ_EIN)
            #s.sendall(request)
            
            #response = s.recv(1024)
            response = mcp.block_read(PSU_ADDR_7BIT, CMD_READ_EIN, num_bytes=7)
            
            if response and len(response) > 0:
                try:
                    result = verify_and_parse_response(response)
                    timestamp = time.strftime("%H:%M:%S.%f")[:-3]
                    print(f"[{timestamp}] P_in = {result['power_watts']:.2f} W | "
                            f"Paccum={result['p_accum']} | N={result['n_samples']} | "
                            f"PEC=OK")
                except Exception as e:
                    print(f"[ERROR] Ошибка парсинга: {e}")
            else:
                print("[WARN] Нет ответа от PSU")
            
            elapsed = time.time() - start_time
            sleep_time = max(0, (POLL_INTERVAL_MS / 1000.0) - elapsed)
            time.sleep(sleep_time)
                
    except KeyboardInterrupt:
        print("\n[HOST] Остановка по запросу пользователя")
    except ConnectionRefusedError:
        print(f"[ERROR] Не удалось подключиться к {HOST}:{PORT}. Запущен ли эмулятор?")
    except Exception as e:
        print(f"[FATAL] Критическая ошибка: {e}")

if __name__ == '__main__':
    run_host_poller()