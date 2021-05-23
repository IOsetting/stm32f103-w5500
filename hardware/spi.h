#ifndef __SPI_H
#define __SPI_H
#include "stm32f10x.h"
 				  	    													  
void SPI1_Init(void);			 //初始化SPI口
void SPI1_SetSpeed(u8 SpeedSet); //设置SPI速度   
void SPI1_WriteByte(u8 TxData);
u8 SPI1_ReadByte(void);
void register_wizchip(void);
void W5500_GPIO_Init(void);
#endif














