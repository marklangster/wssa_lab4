#include "FXOS8700CQ.h"
#include <math.h>

//******************************************************************************
// Public Function Definitions
//------------------------------------------------------------------------------
#define FXOS8700CQ_WHO_AM_I_VAL 0xC7

//------------------------------------------------------------------------------
// FXOS8700CQ(): Initialize configuration parameters
//------------------------------------------------------------------------------
FXOS8700CQ::FXOS8700CQ() {
		magODR = MODR_100HZ; // Magnetometer data/sampling rate
		magOSR = MOSR_5;     // Choose magnetometer oversample rate

    pinMode(CS_PIN, OUTPUT);        // Select the GPIO Pin 51 as SPI Chip Select
    digitalWrite(CS_PIN, HIGH);     // Set Pin to high (Active Low)
}

//------------------------------------------------------------------------------
// writeReg(): Writes to a register
//------------------------------------------------------------------------------
void FXOS8700CQ::writeReg(uint8_t reg, uint8_t data) {
  spi_write_cmd(reg, data);
}

//------------------------------------------------------------------------------
// readReg(): Reads from a register
//------------------------------------------------------------------------------
uint8_t FXOS8700CQ::readReg(uint8_t reg) {
  return spi_read_cmd(reg);
}

//------------------------------------------------------------------------------
// readMagData(): Read the magnometer X, Y and Z axisdata
//------------------------------------------------------------------------------
void FXOS8700CQ::readMagData() {
  //read X
  magData.x = readReg(FXOS8700CQ_M_OUT_X_LSB); //read lower 8 bits
  magData.x |= readReg(FXOS8700CQ_M_OUT_X_MSB) << 8;  //read upper 8 bits
  //read Y
  magData.y = readReg(FXOS8700CQ_M_OUT_Y_LSB); //read lower 8 bits
  magData.y |= readReg(FXOS8700CQ_M_OUT_Y_MSB) << 8;  //read upper 8 bits
  //read Z
  magData.z = readReg(FXOS8700CQ_M_OUT_Z_LSB); //read lower 8 bits
  magData.z |= readReg(FXOS8700CQ_M_OUT_Z_MSB) << 8;  //read upper 8 bits
}

//------------------------------------------------------------------------------
// standby(): Put the FXOS8700CQ into standby mode for writing to registers
//------------------------------------------------------------------------------
void FXOS8700CQ::standby() {
  uint8_t current_config = readReg(FXOS8700CQ_CTRL_REG1);
  writeReg(FXOS8700CQ_CTRL_REG1, current_config & 0xFE); //reset active bit to be in standby mode so we can change this register safely

}

//------------------------------------------------------------------------------
// active(): Put the FXOS8700CQ into active mode to output data
//------------------------------------------------------------------------------
void FXOS8700CQ::active() {
  uint8_t current_config = readReg(FXOS8700CQ_CTRL_REG1);
  writeReg(FXOS8700CQ_CTRL_REG1, current_config | 0x1); //set active mode bit to resume operation

}

//------------------------------------------------------------------------------
// init(): Initialize the magnetometer
//         This function will put the magnetometer in standby mode, modify the 
//         registers that put the device in mag-only mode, set the correct data
//         rate (ODR) and oversampling rate (OSR) for the magnetometer and put
//         it back in active mode
//------------------------------------------------------------------------------
void FXOS8700CQ::init() { 
  
  standby();

  //Write CTRL_REG1 for ODR
  uint8_t current_config = readReg(FXOS8700CQ_CTRL_REG1);
  current_config |= magODR << 3; // set output data rate bits
  //datasheet does not indicate it needs to be in standby mode to set magnetometer ctrl regs
  writeReg(FXOS8700CQ_CTRL_REG1, current_config);

  //Write M_CTRL_REG1 for OSR, mag only mode
  current_config = readReg(FXOS8700CQ_M_CTRL_REG1);
  current_config &= 0xE0; //reset lower 5 bits
  current_config |= (magOSR << 2) | 0x1; // set bits for over sampling rate, and set only magnetometer to be active
  writeReg(FXOS8700CQ_M_CTRL_REG1, current_config);

  active();
  
}
//------------------------------------------------------------------------------
// checkWhoAmI(): Check the whoAmI register
//------------------------------------------------------------------------------
void FXOS8700CQ::checkWhoAmI(void) {
  whoAmIData = readReg(FXOS8700CQ_WHO_AM_I);
  if (whoAmIData == FXOS8700CQ_WHO_AM_I_VAL) 
    SerialUSB.println("Who Am I check passed");
  else {
    SerialUSB.print("Who Am I check failed: ");
    SerialUSB.println(whoAmIData, HEX);
  }
}

//*****************************************************************************
