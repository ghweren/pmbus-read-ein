#ifndef PMBUS_READ_EIN_H
#define PMBUS_READ_EIN_H

#include <stdint.h>

#define PSU_ADDR_7BIT 0x58U
#define PSU_WRITE_ADDR ((PSU_ADDR_7BIT << 1) | 0x00U) // 0xB0
#define PSU_READ_ADDR ((PSU_ADDR_7BIT << 1) | 0x01U)  // 0xB1

#define CMD_READ_EIN 0x86U

#define MAX_BLOCK_SIZE 255U

typedef struct {
    uint16_t p_accum;
    uint32_t n_samples;
    uint8_t byte_count;
    uint8_t pec;
    uint8_t raw_data[5];
} read_ein_response_t;

#ifdef __cplusplus
extern "C" {
#endif

int pmbus_prepare_read_ein_transaction(uint8_t* tx_buf, uint8_t* rx_buf);

int pmbus_parse_read_ein_response(const uint8_t* rx_buf, read_ein_response_t* resp);

#ifdef __cplusplus
}
#endif

#endif