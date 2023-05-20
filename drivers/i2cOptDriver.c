/**************************************************************************************************
*  Filename:       i2cOptDriver.c
*  By:             Jesse Haviland
*  Created:        1 February 2019
*  Revised:        23 March 2019
*  Revision:       2.0
*
*  Description:    i2c Driver for use with opt3001.c and the TI OP3001 Optical Sensor
*************************************************************************************************/



// ----------------------- Includes -----------------------
#include "i2cOptDriver.h"
#include "inc/hw_memmap.h"
#include "driverlib/i2c.h"
#include "utils/uartstdio.h"
#include "driverlib/sysctl.h"
#include <ti/drivers/I2C.h>
#include <xdc/runtime/system.h>
#include "Board.h"
I2C_Handle i2c;
    I2C_Params i2cParams;

bool init_i2c0(void){

    /* Create and Open I2C port*/
    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;
    i2c = I2C_open(Board_I2C_MY_SENSOR, &i2cParams);
    if (i2c == NULL) {
        System_abort("Error Initializing I2C\n");
        return false;
    }
    else {
        System_printf("I2C Initialized!\n");
        return true;
    }

}

/*
 * Sets slave address to ui8Addr
 * Puts ui8Reg followed by two data bytes in *data and transfers
 * over i2c
 */
bool writeI2C(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *data)
{
        uint8_t txBuffer[3];
        I2C_Transaction i2cTransaction;

        i2cTransaction.slaveAddress = ui8Addr;
        i2cTransaction.writeBuf = txBuffer;
        i2cTransaction.writeCount = 2;
        i2cTransaction.readCount = 0;

      //  txBuffer[0] = ui8Reg;
//        txBuffer[2] = *data >> 8;
//        txBuffer[1] = *data & 0xFF;
        txBuffer[0] = data[1];
        txBuffer[1] = data[0];
        bool status = I2C_transfer(i2c, &i2cTransaction);
        return status;
}



/*
 * Sets slave address to ui8Addr
 * Writes ui8Reg over i2c to specify register being read from
 * Reads three bytes from i2c slave. The third is redundant but
 * helps to flush the i2c register
 * Stores first two received bytes into *data
 */
bool readI2C(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *data)
{
    uint8_t txBuffer[1];
    uint8_t rxBuffer[2];
    I2C_Transaction i2cTransaction;

    txBuffer[0] = ui8Reg;

    i2cTransaction.slaveAddress = ui8Addr;
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 2;

    bool status = I2C_transfer(i2c, &i2cTransaction);

    data[0] = rxBuffer[0];
    data[1] = rxBuffer[1];

    return status;
}


