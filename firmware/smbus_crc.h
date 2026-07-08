#ifndef SMBUS_CRC_H
#define SMBUS_CRC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t smbus_calc_crc8(const uint8_t* data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif