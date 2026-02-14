
#include <cstdint>
namespace MPU6050_Regs {

// 1. The Register Addresses
enum Reg : uint8_t {

  SELF_TEST_X_ACCEL = 0x0D,
  SELF_TEST_Y_ACCEL = 0x0E,
  SELF_TEST_Z_ACCEL = 0x0F,
  SELF_TEST_A = 0x10,

  SMPLRT_DIV = 0x19,
  MPU_CONFIG = 0x1A,
  GYRO_CONFIG = 0x1B,
  ACCEL_CONFIG = 0x1C,

  FIFO_ENANBLE = 0x23,
  I2C_MST_CTRL = 0x24,
  I2C_SLV0_ADDR = 0x25,
  I2C_SLV0_REG = 0x26,
  I2C_SLV0_CTRL = 0x27,
  I2C_SLV1_ADDR = 0x28,
  I2C_SLV1_REG = 0x29,
  I2C_SLV1_CTRL = 0x2A,
  I2C_SLV2_ADDR = 0x2B,
  I2C_SLV2_REG = 0x2C,
  I2C_SLV2_CTRL = 0x2D,
  I2C_SLV3_ADDR = 0x2E,
  I2C_SLV3_REG = 0x2F,
  I2C_SLV3_CTRL = 0x30,
  I2C_SLV4_ADDR = 0x31,
  I2C_SLV4_REG = 0x32,
  I2C_SLV4_DO = 0x33,
  I2C_SLV4_CTRL = 0x34,
  I2C_SLV4_DI = 0x35,
  I2C_MST_STATUS = 0x36,
  INT_PIN_CFG = 0x37,
  INTERRUPT_ENABLE = 0x38,
  INT_STATUS = 0x3A,
  ACCEL_XOUT_H = 0x3B,
  ACCEL_XOUT_L = 0x3C,
  ACCEL_YOUT_H = 0x3D,
  ACCEL_YOUT_L = 0x3E,
  ACCEL_ZOUT_H = 0x3F,
  ACCEL_ZOUT_L = 0x40,
  TEMP_OUT_H = 0x41,
  TEMP_OUT_L = 0x42,
  GYRO_XOUT_H = 0x43,
  GYRO_XOUT_L = 0x44,
  GYRO_YOUT_H = 0x45,
  GYRO_YOUT_L = 0x46,
  GYRO_ZOUT_H = 0x47,
  GYRO_ZOUT_L = 0x48,
  I2C_MST_DELAY_CTRL = 0x67,
  SIGNAL_PATH_RESET = 0x68,
  USER_CTRL = 0x6A,
  PWR_MGMT_1 = 0x6B, // Device defaults to the SLEEP mode
  PWR_MGMT_2 = 0x6C,
  FIFO_COUNTH = 0x72,
  FIFO_COUNTL = 0x73,
  FIFO_R_W = 0x74,
  WHO_AM_I = 0x75, // Should return 0x68

  DEFAULT_ADDRESS = 0x68,
};

namespace PWR_MGMT_1_FLAG {
enum flag : uint8_t {
  DEVICE_RESET = 0b1000'0000,
  SLEEP = 0b0100'0000,
  CYCLE = 0b0010'0000,
  TEMP_DIS = 0b0000'1000,
  CLKSEL_INTERNAL = 0,
  CLKSEL_PLL_X = 1,
  CLKSEL_PLL_Y = 2,
  CLKSEL_PLL_Z = 3,
  CLKSEL_PLL_EXTERNAL_32_768K = 4,
  CLKSEL_PLL_EXTERNAL_19_2M = 5,

};
};

namespace PWR_MGMT_2_FLAG {
enum flag : uint8_t {
  LP_WAKE_CTRL_1_25K = 0,
  LP_WAKE_CTRL_5H = 1,
  LP_WAKE_CTRL_20H = 2,
  LP_WAKE_CTRL_40H = 3,
  STBY_XA = 0b0000'0000, // When set to 1, this bit puts the X axis
                         // accelerometer into standby mode.
  STBY_YA = 0b0000'0001, // When set to 1, this bit puts the Y axis
                         // accelerometer into standby mode.
  STBY_ZA = 0b0000'0010, // When set to 1, this bit puts the Z axis
                         // accelerometer into standby mode.
  STBY_XG = 0b0000'0100, // When set to 1, this bit puts the X axis gyroscope
                         // into standby mode.
  STBY_YG = 0b0000'1000, // When set to 1, this bit puts the Y axis gyroscope
                         // into standby mode.
  STBY_ZG = 0b0001'0000, // When set to 1, this bit puts the Z axis gyroscope
                         // into standby mode.

};

}

namespace USER_CTRL_FLAG {
enum flag : uint8_t {
  SIG_COND_RESET = 0b0000'0001,
  I2C_MST_RESET = 0b0000'0010,
  FIFO_RESET = 0b0000'0100,
  I2C_IF_DIS = 0b0001'0000,
  I2C_MST_EN = 0b0010'0000,
  FIFO_EN = 0b0100'0000,
};

}

namespace GYRO_CONFIG_FLAG {
enum flag : uint8_t {
  ZG_ST = 0b0010'0000,
  YG_ST = 0b0100'0000,
  XG_ST = 0b1000'0000,
  FS_SEL_250 = 0b0000'0000,
  FS_SEL_500 = 0b0000'1000,
  FS_SEL_1000 = 0b0001'0000,
  FS_SEL_2000 = 0b0001'1000,

};

}

namespace ACCEL_CONFIG_FLAG {
enum flag : uint8_t {
  ZA_ST = 0b0010'0000,
  YA_ST = 0b0100'0000,
  XA_ST = 0b1000'0000,
  AFS_SEL_2G = 0b0000'0000,
  AFS_SEL_4G = 0b0000'1000,
  AFS_SEL_8G = 0b0001'0000,
  AFS_SEL_16G = 0b0001'1000,

};

} // namespace ACCEL_CONFIG_FLAG

namespace MPU_CONFIG_FLAG {
enum flag : uint8_t {
  EXT_SYNC_SET_INPUT_DISABLED = 0b0000'0000,
  EXT_SYNC_SET_TEMP_OUT_L = 0b0000'1000,
  EXT_SYNC_SET_GYRO_XOUT_L = 0b0001'0000,
  EXT_SYNC_SET_GYRO_YOUT_L = 0b0001'1000,
  EXT_SYNC_SET_GYRO_ZOUT_L = 0b0010'0000,
  EXT_SYNC_SET_ACCEL_XOUT_L = 0b0010'1000,
  EXT_SYNC_SET_ACCEL_YOUT_L = 0b0011'0000,
  EXT_SYNC_SET_ACCEL_ZOUT_L = 0b0011'1000,

  DLPF_CFG0 = 0,
  DLPF_CFG1 = 1,
  DLPF_CFG2 = 2,
  DLPF_CFG3 = 3,
  DLPF_CFG4 = 4,
  DLPF_CFG5 = 5,
  DLPF_CFG6 = 6,

};

}

namespace FIFO_EN_FLAGS {

enum flag : uint8_t {
  TEMP_FIFO_EN = 0b1000'0000,
  XG_FIFO_EN = 0b0100'0000,
  YG_FIFO_EN = 0b0010'0000,
  ZG_FIFO_EN = 0b0001'0000,
  ACCEL_FIFO_EN = 0b0000'1000,
  SLV2_FIFO_EN = 0b0000'0100,
  SLV1_FIFO_EN = 0b0000'0010,
  SLV0_FIFO_EN = 0b0000'0001,
};
}

namespace INT_PIN_CFG_FLAG {

enum flag : uint8_t {
  I2C_BYPASS_EN = 0b0000'0010,
  FSYNC_INT_EN = 0b0000'0100,
  FSYNC_INT_LEVEL = 0b0000'1000,
  INT_RD_CLEAR = 0b0001'0000,
  LATCH_INT_EN = 0b0010'0000,
  INT_OPEN = 0b0100'0000,
  INT_LEVEL = 0b1000'0000,
};
}

namespace INT_ENABLE_FLAG {
enum flag : uint8_t {
  FIFO_OFLOW_EN = 0b0001'0000,
  I2C_MST_INT_EN = 0b0000'1000,
  DATA_RDY_EN = 0b0000'0001,
};

}

} // namespace MPU6050_Regs
