#include <stdint.h>
#include <stdio.h>
#include "pmbus_read_ein.h"

// Добавлено для stm32
/**************************************************************************************************/
#ifdef SIMULATION
typedef struct {
    int dummy;
} I2C_HandleTypeDef;
#define HAL_OK 0
#define HAL_I2C_Master_TransmitReceive(hi2c, addr, tx, txlen, rx, rxlen, timeout) HAL_OK
extern I2C_HandleTypeDef hi2c1;
#else
#include "stm32f4xx_hal.h"
extern I2C_HandleTypeDef hi2c1;
#endif

static uint32_t last_tick_ms = 0;

void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_I2C1_Init(void);

/**************************************************************************************************/

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();

    uint8_t tx_buf[4];
    uint8_t rx_buf[7];
    read_ein_response_t ein_resp;

    printf("PMBus READ_EIN Monitor Started\r\n");
    printf("Target: 0xB0h, Cmd: 0x86h, Interval: 100ms\r\n\r\n");

    while (1) {
        uint32_t now = HAL_GetTick();

        if ((now - last_tick_ms) >= 100) {
            last_tick_ms = now;

            pmbus_prepare_read_ein_transaction(tx_buf, rx_buf);

            HAL_StatusTypeDef status = HAL_I2C_Master_TransmitReceive(&hi2c1, PSU_ADDR_7BIT << 1,
                                                                      tx_buf, 2, rx_buf, 7, 100);

            if (status == HAL_OK) {
                if (pmbus_parse_read_ein_response(rx_buf, &ein_resp) == 0) {
                    float power_avg = 0.0f;
                    if (ein_resp.n_samples > 0) {
                        power_avg = (float)ein_resp.p_accum / (float)ein_resp.n_samples;
                    }

                    printf("[%lu ms] P_in = %.2f W | Acc=%u | N=%lu | PEC=OK\r\n", now, power_avg,
                           ein_resp.p_accum, ein_resp.n_samples);
                } else {
                    printf("[%lu ms] ERROR: PEC Check Failed!\r\n", now);
                }
            } else {
                printf("[%lu ms] ERROR: I2C Transfer Failed (Status: %d)\r\n", now, status);
            }
        }
    }
}