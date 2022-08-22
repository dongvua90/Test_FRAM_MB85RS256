/*
 * MB85RS256.c
 *
 *  Created on: Aug 22, 2022
 *      Author: goku
 */
#include "MB85RS256.h"

#define MB85RS_CS_LOW 	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET)
#define MB85RS_CS_HIGH 	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET)

// OP-CODE
#define WREN       0x06		// Set Write Enable Latch
#define WRDI       0x04  	// Reset Write Enable Latch
#define RDSR       0x05		// Read Status Register
#define WRSR       0x01		// Write Status Register
#define READ       0x03		// Read Memory Code
#define WRITE      0x02		// Write Memory Code
#define RDID       0x9F
#define SLEEP      0xB9

uint8_t tik=0;
uint8_t Mb85rs_dataTx[50];
uint8_t Mb85rs_dataRx[50];
uint8_t data_rx[35];
uint8_t data_tx[35];
uint8_t Mb85rs_cmd[3];
uint8_t cmd=0;
extern SPI_HandleTypeDef hspi1;
HAL_StatusTypeDef status;

// Thiết lập lại trạng thái Write ( bảo vệ , không cho phép Write)
void WriteProtect(){
	uint8_t cmd[1];
	cmd[0]= WRDI; // Reset Write Enable Latch
	MB85RS_CS_LOW;
	status = HAL_SPI_Transmit(&hspi1, cmd, 1, 1);
	MB85RS_CS_HIGH;
}
uint8_t ReadStatus(){
	uint8_t reg[2],cmd[2];
	cmd[0] = RDSR;// Read Status Register
	MB85RS_CS_LOW;
	HAL_SPI_TransmitReceive(&hspi1,cmd, reg, 2, 1);
	MB85RS_CS_HIGH;
	return reg[1];
}
void WriteEnable(){
	uint8_t cmd[1];
	cmd[0]=WREN;
	MB85RS_CS_LOW;
	status = HAL_SPI_Transmit(&hspi1, cmd, 1, 1);
	MB85RS_CS_HIGH;
}

void MB85RS_write(unsigned int address, unsigned char *buffer, unsigned int size)
{
	Mb85rs_dataTx[0] = WRITE;
	Mb85rs_dataTx[1] = (char)(address >> 8);
	Mb85rs_dataTx[2] = (char)(address);
	for(int i=0;i<size;i++){
		Mb85rs_dataTx[3+i] = buffer[i];
	}
	WriteEnable();
	MB85RS_CS_LOW;
	HAL_SPI_Transmit(&hspi1, Mb85rs_dataTx, size+3, 10);
	MB85RS_CS_HIGH;
	WriteProtect();
}
void MB85RS_read(unsigned int address, unsigned char *buffer, unsigned int size)
{
	uint8_t cmd[3];
	cmd[0]=READ;
	cmd[1]=(char)(address >> 8);
	cmd[2] = (char)(address);
	MB85RS_CS_LOW;
	HAL_SPI_Transmit(&hspi1, cmd, 3, 10);
	HAL_SPI_Receive(&hspi1, buffer, size, 10);
	MB85RS_CS_HIGH;
}

void MB85RS_erase()
{
	uint8_t cmd[3];
	uint8_t datanull[128];
	//setStatus(0x00);
	cmd[0]=WRITE;
	cmd[1] = 0;
	cmd[2] = 0;
	WriteEnable();
	MB85RS_CS_LOW;
	for(int i=0;i<128;i++){
		datanull[i]=0;
	}
	HAL_SPI_Transmit(&hspi1, cmd, 3, 100);
	for(int i = 0; i <= 256; i++)
	{
		HAL_SPI_Transmit(&hspi1, datanull, 128, 100);
	}
	MB85RS_CS_HIGH;
	WriteProtect();
}


uint8_t reg=0;

void StartTaskFram(void *argument){
	osDelay(100);
	while(1){
		if(cmd==1){
			WriteProtect();
			cmd=0;
		}else if(cmd==2){
			WriteEnable();
			cmd=0;
		}else if(cmd==3){ // transmit
			for(int i=0;i<25;i++){
				data_tx[i]=i;
			}
			MB85RS_write(1, data_tx, 25);
			cmd=0;
		}else if(cmd==4){ //receiver
			MB85RS_read(1, data_rx, 25);
			cmd=0;
		}else if(cmd==5){
			for(int i=0;i<25;i++){
				data_tx[i]=0;
			}
			MB85RS_write(1, data_tx, 25);
			cmd=0;
		}else if(cmd==6){
			reg=ReadStatus();
			cmd=0;
		}else if(cmd==7){
			MB85RS_erase();
			cmd=0;
		}
		osDelay(100);
	}

}
