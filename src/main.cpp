#include <Arduino.h>
#include <AccelStepper.h>
#include <U8g2lib.h>
#include <Wire.h>

// === 1. 硬件引脚配置 ===
// ⚠️ 硬件跳线注意：DRV8825的 MS1, MS2, MS3 接高电平(5V/3.3V) -> 对应 1/8 细分
#define MOTOR_STEPS 200
#define LDR_DO      PA0     //定义光敏电阻数字开关量输出引脚
#define LDR_AO      PA1     //定义光敏模拟电压输出引脚
#define EN_PIN      PB5     //定义电机使能引脚
#define STEP_PIN    PB8     //定义电机步进引脚
#define DIR_PIN     PB9     //定义电机方向引脚
#define LED_PIN     PA15    //定义genericSTM32F103C8外置LED引脚

// === 2. 运行参数 (核心修改) ===
#define TARGET_RPM   200     // 目标转速 200 RPM
#define MICROSTEPS   8       // ⚠️ 关键：改为1/8细分！(需配合硬件跳线)

// === 3. 布局设置 ===
#define MARGIN_X 0 
#define COL_2_X 64 

// === 4. 对象初始化 ===
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);
// genericSTM32F103C8 I2C1: SCL=PB6, SDA=PB7
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// === 5. 全局变量 ===
long pps_speed = 0;           // 实时脉冲速度
bool is_running = false;      // 电机运行状态标志
bool is_forward = true;       // 当前运动方向 (true: 正转, false: 反转)
bool last_light_state = false; // 上一次光照状态

// PROGMEM字符串，减少内存碎片
const char PROGMEM STR_VOLTAGE[] = "V:5-12V";
const char PROGMEM STR_DIV[] = "Div:1/8";
const char PROGMEM STR_RPM[] = "R:";
const char PROGMEM STR_HZ[] = "Hz:";
const char PROGMEM STR_MODE[] = "Mode: ";
const char PROGMEM STR_CONTINUOUS[] = "Continuous";
const char PROGMEM STR_LED[] = "L:";
const char PROGMEM STR_ON[] = "ON";
const char PROGMEM STR_OFF[] = "OFF";
const char PROGMEM STR_STOPPED[] = "MOTOR STOPPED";
const char PROGMEM STR_NO_LIGHT[] = "No Light Detected";
const char PROGMEM STR_LDR[] = "LDR Analog:";
const char PROGMEM STR_WAITING[] = "Next: ";
const char PROGMEM STR_FORWARD[] = "Forward";
const char PROGMEM STR_REVERSE[] = "Reverse";

// ---------------------------------------------------------
// OLED 绘制函数 (使用PROGMEM字符串优化)
// ---------------------------------------------------------
void drawStatusScreen(const char* dir_title, bool led_on) {
    u8g2.clearBuffer();
    
    // 标题栏
    u8g2.setFont(u8g2_font_7x14B_tf); 
    u8g2.setCursor(MARGIN_X, 12);
    u8g2.print(dir_title);
    u8g2.drawLine(0, 15, 128, 15);

    // 数据区
    u8g2.setFont(u8g2_font_6x10_tf); 

    // Row 1
    u8g2.setCursor(MARGIN_X, 26); u8g2.print(STR_VOLTAGE);
    u8g2.setCursor(COL_2_X, 26);  u8g2.print(STR_DIV);

    // Row 2
    u8g2.setCursor(MARGIN_X, 38); u8g2.print(STR_RPM); u8g2.print(TARGET_RPM); 
    u8g2.setCursor(COL_2_X, 38);  u8g2.print(STR_HZ); u8g2.print(pps_speed);

    // Row 3
    u8g2.setCursor(MARGIN_X, 50); u8g2.print("A:"); u8g2.print(2000);
    u8g2.setCursor(COL_2_X, 50);  u8g2.print(STR_LED); 
    if (led_on) {
        u8g2.print(STR_ON);
    } else {
        u8g2.print(STR_OFF);
    }

    // Row 4
    u8g2.setCursor(MARGIN_X, 62);
    u8g2.print(STR_MODE); 
    u8g2.print(STR_CONTINUOUS);

    u8g2.sendBuffer();
}

// ---------------------------------------------------------
// Setup
// ---------------------------------------------------------
void setup() {
    // 1. OLED Init
    u8g2.begin();
    
    // 2. Pin Init
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH); // BluePill LED LOW is ON usually, adjust if needed
    
    // 3. LDR Sensor Init
    pinMode(LDR_DO, INPUT);  // 数字输出引脚
    pinMode(LDR_AO, INPUT);  // 模拟输入引脚
    
    // 3. Stepper Init
    // 设置使能引脚
    pinMode(EN_PIN, OUTPUT);
    digitalWrite(EN_PIN, LOW); // 使能驱动器
    
    // 设置最大速度（使用runSpeed()时不需要加速度）
    long max_speed = TARGET_RPM * MOTOR_STEPS * MICROSTEPS / 60;
    stepper.setMaxSpeed(max_speed);
    
    // 4. 计算实时脉冲频率 Hz
    pps_speed = (long)(TARGET_RPM * MOTOR_STEPS * MICROSTEPS) / 60;
    
}

// 显示控制选项
#define DISABLE_OLED 0  // 设置为1可禁用OLED显示以测试是否是显示导致的卡顿

// ---------------------------------------------------------
// 独立的显示更新函数
// ---------------------------------------------------------
void updateDisplay(bool show_running, int ldr_value) {
    #if DISABLE_OLED
        return;  // 如果禁用OLED，直接返回
    #endif
    
    u8g2.clearBuffer();
    
    if (show_running) {
        // 恢复完整的运行状态显示
        u8g2.setFont(u8g2_font_7x14B_tf); 
        if (is_forward) {
            u8g2.setCursor(MARGIN_X, 12);
            u8g2.print(">> FWD 300RPM");
        } else {
            u8g2.setCursor(MARGIN_X, 12);
            u8g2.print("<< REV 300RPM");
        }
        u8g2.drawLine(0, 15, 128, 15);

        u8g2.setFont(u8g2_font_6x10_tf); 

        // 恢复参数显示
        u8g2.setCursor(MARGIN_X, 26); u8g2.print(STR_VOLTAGE);
        u8g2.setCursor(COL_2_X, 26);  u8g2.print(STR_DIV);

        u8g2.setCursor(MARGIN_X, 38); u8g2.print(STR_RPM); u8g2.print(TARGET_RPM); 
        u8g2.setCursor(COL_2_X, 38);  u8g2.print(STR_HZ); u8g2.print(pps_speed);

        u8g2.setCursor(MARGIN_X, 50); u8g2.print("A:"); u8g2.print(2000);
        u8g2.setCursor(COL_2_X, 50);  u8g2.print(STR_LED); u8g2.print(STR_ON);

        u8g2.setCursor(MARGIN_X, 62);
        u8g2.print(STR_MODE); 
        u8g2.print(STR_CONTINUOUS);
    } else {
        // 显示停止状态（保持不变）
        u8g2.setFont(u8g2_font_7x14B_tf);
        u8g2.setCursor(20, 12); u8g2.print(STR_STOPPED);
        u8g2.drawLine(0, 15, 128, 15);
        
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.setCursor(0, 30); u8g2.print("No Light Detected");
        u8g2.setCursor(0, 42); u8g2.print("LDR Analog:"); u8g2.print(ldr_value);
        u8g2.setCursor(0, 54); u8g2.print("Next: "); 
        if (is_forward) {
            u8g2.print("Forward");
        } else {
            u8g2.print("Reverse");
        }
    }
    
    u8g2.sendBuffer();
}

// ---------------------------------------------------------// Loop// ---------------------------------------------------------
void loop() {
    static unsigned long last_sensor_check = 0;
    int ldr_analog = 0;
    
    // 核心电机控制 - 最高优先级
    if (is_running) {
        stepper.runSpeed();
        
        // 定期检查传感器状态（每50ms一次），但不更新显示
        if (millis() - last_sensor_check > 50) {
            bool has_light = !digitalRead(LDR_DO); // 假设光照时DO输出LOW
            
            // 检测光照状态变化
            if (has_light != last_light_state) {
                last_light_state = has_light;
                
                if (!has_light) {
                    // 光照消失，停止电机
                    stepper.setSpeed(0);
                    is_running = false;
                    digitalWrite(LED_PIN, HIGH); // 关闭LED
                    
                    // 切换方向标志
                    is_forward = !is_forward;
                    
                    // 更新显示
                    ldr_analog = analogRead(LDR_AO);
                    updateDisplay(false, ldr_analog);
                }
            }
            
            last_sensor_check = millis();
        }
        
        return; // 电机运行时，只执行必要的检查
    }
    
    // 电机停止时的操作
    
    // 定期检查传感器状态
    if (millis() - last_sensor_check > 50) {
        // 检测光照状态
        bool has_light = !digitalRead(LDR_DO); // 假设光照时DO输出LOW
        ldr_analog = analogRead(LDR_AO);   // 读取模拟值
        
        // 检测光照状态变化
        if (has_light != last_light_state) {
            last_light_state = has_light;
            
            if (has_light) {
                // 有光照，启动电机
                digitalWrite(LED_PIN, LOW); // 点亮LED
                
                // 设置电机速度（正反转）
                long max_speed = TARGET_RPM * MOTOR_STEPS * MICROSTEPS / 60;
                if (is_forward) {
                    stepper.setSpeed(max_speed);
                } else {
                    stepper.setSpeed(-max_speed);
                }
                
                // 更新显示
                updateDisplay(true, 0);
                
                is_running = true;
            }
        }
        
        last_sensor_check = millis();
    }
}