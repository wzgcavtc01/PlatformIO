#ifndef __OLED_H
#define __OLED_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// 基础功能函数
void OLED_Init(void);
void OLED_Clear(void);
void OLED_SetCursor(uint8_t Y, uint8_t X);

// 显示函数
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr);
void OLED_ShowString(uint8_t Line, uint8_t Column, uint8_t *String);
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);

// 辅助函数
uint32_t OLED_Pow(uint32_t X, uint32_t Y);

#ifdef __cplusplus
}
#endif

#endif
