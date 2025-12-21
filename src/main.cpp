#include <Arduino.h>
#include "simple_oled.h"

// 定义输出引脚
const int outputPin = PA0;    // STM32 3.3V输出引脚
const int statusLed = PC13;  // 板载LED用于状态指示

// OLED引脚定义
const int oledSDA = PA4;      // OLED SDA引脚
const int oledSCL = PA5;      // OLED SCL引脚

// 功能标志
#define VERSION_WITH_PA0_WORKING 1
#define ENABLE_OLED_DISPLAY 1

void setup() {
  // 初始化串口通信
  Serial.begin(115200);
  delay(1000);
  
  // 设置引脚为输出模式
  pinMode(outputPin, OUTPUT);
  pinMode(statusLed, OUTPUT);
  
  // 初始状态：PA0低电平，板载LED熄灭
  digitalWrite(outputPin, LOW);
  digitalWrite(statusLed, HIGH); // PC13低电平点亮，所以HIGH是熄灭
  
  Serial.println("=== STM32 Pin Status Monitor ===");
  Serial.println("Output: PA0");
  Serial.println("Status LED: PC13");
  Serial.println("OLED: PA4(SDA), PA5(SCL)");
  Serial.println("Init Complete!");
  Serial.println();
  
#if ENABLE_OLED_DISPLAY
  // 强制尝试初始化OLED（无串口调试版本）
  SimpleOLED_Init();
  
  // 显示连接检查信息
  SimpleOLED_ShowCheck();
  delay(3000); // 延长显示时间以便查看
  
  // 显示PA0功能确认信息
  SimpleOLED_Clear();
  SimpleOLED_ShowString(0, 0, "PA0: 8s Cycle");
  SimpleOLED_ShowString(0, 1, "5s HIGH 3s LOW");
  SimpleOLED_ShowString(0, 2, "3.3V Output");
  SimpleOLED_ShowString(0, 3, "Starting Main");
  delay(2000);
#endif
  
  // 启动指示：板载LED闪烁3次表示系统就绪
  for(int i = 0; i < 3; i++) {
    digitalWrite(statusLed, LOW);  // 点亮
    Serial.print("LED ON ");
    delay(200);
    digitalWrite(statusLed, HIGH); // 熄灭
    Serial.println("LED OFF");
    delay(200);
  }
  
  digitalWrite(statusLed, HIGH); // 熄灭，准备进入主循环
  Serial.println("System Ready - Starting main loop");
  Serial.println("=================================");
}

void loop() {
  static int counter = 0;
  static unsigned long lastUpdate = 0;
  static unsigned long stateStartTime = 0;
  static bool isHighState = false;
  unsigned long currentTime = millis();
  
  // 状态切换逻辑（8秒周期：5秒高电平，3秒低电平）
  if (!isHighState && (currentTime - lastUpdate >= 3000)) {
    // 切换到高电平状态
    isHighState = true;
    lastUpdate = currentTime;
    stateStartTime = currentTime;
    digitalWrite(outputPin, HIGH);
    Serial.println("\n=== STATE CHANGED TO HIGH ===");
  } else if (isHighState && (currentTime - lastUpdate >= 5000)) {
    // 切换到低电平状态
    isHighState = false;
    lastUpdate = currentTime;
    stateStartTime = currentTime;
    digitalWrite(outputPin, LOW);
    Serial.println("\n=== STATE CHANGED TO LOW ===");
  }
  
  // LED闪烁效果
  static unsigned long lastBlink = 0;
  if (currentTime - lastBlink >= 1000) {
    lastBlink = currentTime;
    digitalWrite(statusLed, !digitalRead(statusLed));
  }
  
  // 每500ms更新状态信息（通过串口输出）
  static unsigned long lastDisplay = 0;
  if (currentTime - lastDisplay >= 500) {
    lastDisplay = currentTime;
    
    // 显示PA0引脚状态
    unsigned long stateDuration = currentTime - stateStartTime;
    unsigned long remainingTime = isHighState ? (5000 - stateDuration) : (3000 - stateDuration);
    
    Serial.print("[");
    Serial.print(millis() / 1000);
    Serial.print("s] PA0: ");
    Serial.print(digitalRead(outputPin) ? "HIGH(3.3V)" : "LOW(0V)");
    Serial.print(" | Status: ");
    Serial.print(isHighState ? "HIGH" : "LOW");
    Serial.print(" | Time: ");
    Serial.print(stateDuration / 1000);
    Serial.print("/");
    Serial.print((stateDuration + remainingTime) / 1000);
    Serial.print("s | Switch in: ");
    Serial.print(remainingTime / 1000);
    Serial.print("s | PC13 LED: ");
    Serial.print(digitalRead(statusLed) ? "OFF" : "ON");
    Serial.print(" | Counter: ");
    Serial.println(counter++);
    
#if ENABLE_OLED_DISPLAY
    // OLED显示状态信息
    char line1[20] = {0}, line2[20] = {0}, line3[20] = {0}, line4[20] = {0};
    
    sprintf(line1, "PA0: %s", digitalRead(outputPin) ? "HIGH " : "LOW  ");
    sprintf(line2, "Time: %2lds/%2lds", stateDuration / 1000, (stateDuration + remainingTime) / 1000);
    sprintf(line3, "Switch: %2lds", remainingTime / 1000);
    sprintf(line4, "Count: %6d", counter);
    
    // 静态显示，减少I2C冲突
    static unsigned long lastOLEDUpdate = 0;
    if(currentTime - lastOLEDUpdate >= 200) {  // 每200ms更新一次OLED
        lastOLEDUpdate = currentTime;
        SimpleOLED_ShowString(0, 0, line1);
        SimpleOLED_ShowString(0, 1, line2);
        SimpleOLED_ShowString(0, 2, line3);
        SimpleOLED_ShowString(0, 3, line4);
    }
#endif
  }
}