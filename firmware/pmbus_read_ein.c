#include "pmbus_read_ein.h"
#include <string.h>
#include "smbus_crc.h"

int pmbus_prepare_read_ein_transaction(uint8_t* tx_buf, uint8_t* rx_buf) {
    if (!tx_buf || !rx_buf)
        return -1;

    tx_buf[0] = PSU_WRITE_ADDR;
    tx_buf[1] = CMD_READ_EIN;

    memset(rx_buf, 0, 7);

    return 0;
}

int pmbus_parse_read_ein_response(const uint8_t* rx_buf, read_ein_response_t* resp) {
    if (!rx_buf || !resp)
        return -1;

    uint8_t byte_count = rx_buf[0];

    if (byte_count != 5) {
        return -1;
    }

    uint8_t pec_input[9];
    pec_input[0] = PSU_WRITE_ADDR;
    pec_input[1] = CMD_READ_EIN;
    pec_input[2] = PSU_READ_ADDR;
    pec_input[3] = byte_count;
    memcpy(&pec_input[4], &rx_buf[1], 5);

    uint8_t calc_pec = smbus_calc_crc8(pec_input, sizeof(pec_input));
    uint8_t recv_pec = rx_buf[6];

    if (calc_pec != recv_pec) {
        return -1;
    }

    resp->p_accum = (uint16_t)rx_buf[1] | ((uint16_t)rx_buf[2] << 8);
    resp->n_samples =
        (uint32_t)rx_buf[3] | ((uint32_t)rx_buf[4] << 8) | ((uint32_t)rx_buf[5] << 16);

    resp->byte_count = byte_count;
    resp->pec = recv_pec;
    memcpy(resp->raw_data, &rx_buf[1], 5);

    return 0;
}