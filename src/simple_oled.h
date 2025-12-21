#ifndef __SIMPLE_OLED_H
#define __SIMPLE_OLED_H

#include <stdint.h>

// 简单的OLED驱动函数声明
void SimpleOLED_Init(void);
void SimpleOLED_Clear(void);
void SimpleOLED_ShowString(uint8_t x, uint8_t y, const char* str);
void SimpleOLED_Test(void);
bool SimpleOLED_Detect(void);
void SimpleOLED_ShowCheck(void);

#endif