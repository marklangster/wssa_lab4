#include <Arduino.h>
#include<SPI.h>
#include<math.h>
#include <stdint.h>

//#define SPI_DEBUG 1

// Chip Select Pin for SPI
#define CS_PIN 51

//data setup on falling edge, data capture on rising edge
#define SPI_MODE SPI_MODE0 
// 1 MHz
#define SPI_HZ 1000000
// Send most significant bit first
#define SPI_ORDER MSBFIRST



//Function Declarations
void spi_write_cmd(uint8_t address, uint8_t tx_data);
uint8_t spi_read_cmd(uint8_t address);
