#ifndef LED_H
#define LED_H

#include <stdint.h>

#define LED_BASE (0x1C010008)	// �����ַҲ�ǹ̶���
#define LED      ((uint8_t *)LED_BASE)

void changeColorOfLED();
#endif // LED_H
