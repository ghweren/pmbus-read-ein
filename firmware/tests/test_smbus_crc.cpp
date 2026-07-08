#include <gtest/gtest.h>
#include "smbus_crc.h"

TEST(SMBusCRC, KnownVectorFromPythonMock) {
    uint8_t data[] = {0xB0, 0x86, 0xB1, 0x05, 0x28, 0x15, 0x27, 0x02, 0x00};
    uint8_t expected_crc = 0xE8;

    EXPECT_EQ(smbus_calc_crc8(data, sizeof(data)), expected_crc);
}

TEST(SMBusCRC, EmptyBuffer) {
    EXPECT_EQ(smbus_calc_crc8(nullptr, 0), 0);
}

TEST(SMBusCRC, SingleByte) {
    uint8_t data[] = {0xFF};
    uint8_t result = smbus_calc_crc8(data, 1);
    EXPECT_NE(result, 0xFF);
}