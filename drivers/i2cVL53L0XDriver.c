//#include "i2cOptDriver.h"
#include "inc/hw_memmap.h"
#include "driverlib/i2c.h"
#include "utils/uartstdio.h"
#include "driverlib/sysctl.h"
#include <ti/drivers/I2C.h>
#include <xdc/runtime/system.h>
#include "Board.h"

#define VL53L0X_DEFAULT_ADDR                0x29
#define VL53L0X_REG_SYSRANGE_START          0x00
#define VL53L0X_REG_RESULT_RANGE_STATUS     0x14

I2C_Handle i2cHandle;
I2C_Params i2cParams;

void VL53L0X_writeByte(uint8_t reg, uint8_t value)
{
    uint8_t buffer[2];
    buffer[0] = reg;
    buffer[1] = value;

    I2C_Transaction transaction;
    transaction.slaveAddress = VL53L0X_DEFAULT_ADDR;
    transaction.writeBuf = buffer;
    transaction.writeCount = 2;
    transaction.readBuf = NULL;
    transaction.readCount = 0;

    I2C_transfer(i2cHandle, &transaction);
}

bool VL53L0X_readByte(uint8_t reg)
{
    uint8_t value;

    I2C_Transaction transaction;
    transaction.slaveAddress = VL53L0X_DEFAULT_ADDR;
    transaction.writeBuf = &reg;
    transaction.writeCount = 1;
    transaction.readBuf = &value;
    transaction.readCount = 1;

    I2C_transfer(i2cHandle, &transaction);

    return true;
}

bool VL53L0X_readWord(uint8_t reg)
{
    uint8_t buffer[2];

    I2C_Transaction transaction;
    transaction.slaveAddress = VL53L0X_DEFAULT_ADDR;
    transaction.writeBuf = &reg;
    transaction.writeCount = 1;
    transaction.readBuf = buffer;
    transaction.readCount = 2;

    I2C_transfer(i2cHandle, &transaction);
    uint16_t result = (buffer[0] << 8) | buffer[1];
    return true;
}

void VL53L0X_init()
{
    I2C_Params_init(&i2cParams);
    i2cHandle = I2C_open(0, &i2cParams);

    // Additional initialization code for VL53L0X if required
}

void VL53L0X_deinit()
{
    I2C_close(i2cHandle);
}
