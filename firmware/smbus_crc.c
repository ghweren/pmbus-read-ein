#include "smbus_crc.h"

uint8_t smbus_calc_crc8(const uint8_t* data, uint16_t len) {
    uint8_t crc = 0;

    for (uint16_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x07;
            } else {
                crc = (crc << 1);
            }
        }
    }
    return crc;
}