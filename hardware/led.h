
#ifndef	__LED_H
#define __LED_H
#include "stm32f10x.h"

void led_init(void);
void led_pa1(u8 sta);
void led_toggle(void);

#endif
