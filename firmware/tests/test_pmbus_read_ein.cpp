#include <gtest/gtest.h>
#include "pmbus_read_ein.h"
#include "smbus_crc.h"

class PMBusReadEinTest : public ::testing::Test {
protected:
    void SetUp() override {
        memset(rx_buf_, 0, sizeof(rx_buf_));
    }
    
    uint8_t rx_buf_[7];
    read_ein_response_t resp_;
};

TEST_F(PMBusReadEinTest, ValidResponseParsing) {
    rx_buf_[0] = 0x05;
    rx_buf_[1] = 0x28; rx_buf_[2] = 0x15;
    rx_buf_[3] = 0x27; rx_buf_[4] = 0x02; rx_buf_[5] = 0x00;
  
    uint8_t pec_input[9] = {0xB0, 0x86, 0xB1, 0x05, 0x28, 0x15, 0x27, 0x02, 0x00};
    rx_buf_[6] = smbus_calc_crc8(pec_input, 9);
    
    EXPECT_EQ(pmbus_parse_read_ein_response(rx_buf_, &resp_), 0);
    EXPECT_EQ(resp_.p_accum, 0x1528);
    EXPECT_EQ(resp_.n_samples, 0x000227);
    EXPECT_EQ(resp_.byte_count, 5);
}

TEST_F(PMBusReadEinTest, InvalidPEC) {
    rx_buf_[0] = 0x05;
    rx_buf_[1] = 0x28; rx_buf_[2] = 0x15;
    rx_buf_[3] = 0x27; rx_buf_[4] = 0x02; rx_buf_[5] = 0x00;
    rx_buf_[6] = 0xFF;
    
    EXPECT_EQ(pmbus_parse_read_ein_response(rx_buf_, &resp_), -1);
}

TEST_F(PMBusReadEinTest, WrongByteCount) {
    rx_buf_[0] = 0x04;
    EXPECT_EQ(pmbus_parse_read_ein_response(rx_buf_, &resp_), -1);
}