#include "led.h"


void led_init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; // PB12
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_SetBits(GPIOB,GPIO_Pin_12);
}

void led_pa1(u8 sta)
{
  if(sta==1) {
    GPIO_ResetBits(GPIOB, GPIO_Pin_12);
  } else {
    GPIO_SetBits(GPIOB, GPIO_Pin_12);
  }
}

void led_toggle(void)
{
  u8 i = GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_12);
  if (i == 0) {
    GPIO_SetBits(GPIOB, GPIO_Pin_12);
  } else {
    GPIO_ResetBits(GPIOB, GPIO_Pin_12);
  }
}
