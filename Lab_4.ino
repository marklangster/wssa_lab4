#include <SPI.h>
#include "FXOS8700CQ.h"
#include <FreeRTOS_ARM.h> 

#define INTERRUPT_PIN 52


FXOS8700CQ sensor;

QueueHandle_t queue;
int dataCollected = 0;

SemaphoreHandle_t processSem;
SemaphoreHandle_t collectSem;

void setup() {
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);
  digitalWrite(8, LOW);

  // Initialize SerialUSB 
  while(!SerialUSB);
  SerialUSB.begin(9600);

  processSem = xSemaphoreCreateCounting(1,0);
  collectSem = xSemaphoreCreateCounting(1,0);
  
  // Initialize SPI
  SPI.begin();

  // Initialize sensor
  sensor = FXOS8700CQ();
  sensor.init();
  delay(10);

  sensor.checkWhoAmI();

  offsetAndInterrupt();
  
  xTaskCreate(collectData, "collectData", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
  xTaskCreate(processData, "processData", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
  vTaskStartScheduler();
  while(1);
}

void loop() {
  
}

void processIntr(){
  noInterrupts();
  xSemaphoreGive(collectSem);
  interrupts();
}
void collectData(void *arg){
  uint8_t value;
  while(1){
    xSemaphoreTake(collectSem, portMAX_DELAY);
    for (int i=0; i < 2; i++){
      sensor.readMagData();
      SerialUSB.print("X value = "); 
      SerialUSB.println(sensor.magData.x);
      
      SerialUSB.print("Y value = "); 
      SerialUSB.println(sensor.magData.y);
      
      SerialUSB.print("Z value = "); 
      SerialUSB.println(sensor.magData.z);
    
      SerialUSB.println("");
    }
    
    xSemaphoreGive(processSem);
  }
}


void processData(void *arg){
  while(1){
    xSemaphoreTake(processSem, portMAX_DELAY);
    dataCollected++;
    SerialUSB.print("Collected data ");
    SerialUSB.print(dataCollected);
    SerialUSB.println("times");
  }
}

void offsetAndInterrupt(){
  uint16_t x = 0;
  uint16_t y = 0;
  uint16_t z = 0;
  
  for (int i = 0; i <5; i++){
    sensor.readMagData();
    x = ((x * i) + sensor.magData.x) / (i+1);
    y = ((y * i) + sensor.magData.y) / (i+1);
    z = ((z * i) + sensor.magData.z) / (i+1);
  }
  
  uint8_t reg_config = 0;
  sensor.writeReg(FXOS8700CQ_M_VECM_CFG, 0); //set magnetic vector configuration. 0x69
  reg_config = 75;
  sensor.writeReg(FXOS8700CQ_M_THS_CFG, 75); //disable magnetic-threshold interrupt. 0x52

  sensor.writeReg(FXOS8700CQ_M_THS_X_MSB, x >> 8); 
  sensor.writeReg(FXOS8700CQ_M_THS_X_LSB, x & 0xFF);

  sensor.writeReg(FXOS8700CQ_M_THS_Y_MSB, y >> 8);
  sensor.writeReg(FXOS8700CQ_M_THS_Y_LSB, y & 0xFF);
  
  sensor.writeReg(FXOS8700CQ_M_THS_Z_MSB, z >> 8);
  sensor.writeReg(FXOS8700CQ_M_THS_Z_LSB, z & 0xFF);

  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), processIntr, FALLING);  

}

