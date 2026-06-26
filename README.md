# PlatformIO - STM32 Stepper Motor Control with Light Sensor

[![PlatformIO](https://img.shields.io/badge/PlatformIO-IDE-orange)](https://platformio.org/)
[![STM32](https://img.shields.io/badge/MCU-STM32F103C8-blue)](https://www.st.com/en/microcontrollers-microprocessors/stm32f103c8.html)
[![Framework](https://img.shields.io/badge/Framework-Arduino-green)](https://www.arduino.cc/)

## 📋 Overview

This project is a **STM32F103C8 (Blue Pill)** based stepper motor control system built with **PlatformIO** and **Arduino framework**. It uses a **DRV8825 stepper driver** to control a bipolar stepper motor, with feedback from a **light-dependent resistor (LDR)** sensor and status display on an **SSD1306 OLED** screen.

## 🎯 Features

- **Light-Activated Motor Control** — Stepper motor starts running when light is detected by the LDR, stops when light disappears
- **Automatic Direction Reversal** — Motor reverses direction each time it restarts after a stop
- **1/8 Microstepping** — DRV8825 configured for 1/8 microstepping (hardware jumpers)
- **Real-Time OLED Display** — SSD1306 128x64 I2C OLED shows:
  - Motor speed (RPM) and pulse frequency (Hz)
  - Current draw (mA)
  - Operating voltage range (5-12V)
  - Microstepping division ratio (1/8)
  - LED status (ON/OFF)
  - Drive mode (Continuous)
- **LED Indicator** — External LED lights up when motor is running
- **Sensor Persistence** — Continuous real-time sensor polling with 50ms interval

## 🔧 Hardware Requirements

| Component | Specification |
|-----------|--------------|
| **MCU** | STM32F103C8 (Blue Pill) |
| **Stepper Driver** | DRV8825 (1/8 microstepping via hardware jumpers) |
| **Stepper Motor** | Bipolar, 200 steps/rev, 5-12V |
| **Light Sensor** | LDR module (digital + analog output) |
| **Display** | SSD1306 128x64 OLED, I2C (SCL=PB6, SDA=PB7) |
| **Indicator** | External LED |
| **Programmer** | ST-Link V2 |

## ⚙️ Pin Configuration

| Pin | Function |
|-----|----------|
| PA0 | LDR Digital Output |
| PA1 | LDR Analog Output |
| PA15 | External LED |
| PB5 | Stepper Driver Enable |
| PB6 | I2C1 SCL (OLED) |
| PB7 | I2C1 SDA (OLED) |
| PB8 | Stepper STEP |
| PB9 | Stepper DIR |

## 📦 Software Dependencies

- **PlatformIO** with `ststm32` platform
- **Arduino framework** for STM32
- **[AccelStepper](https://github.com/waspinator/AccelStepper)** v1.64+ — Stepper motor control library
- **[U8g2](https://github.com/olikraus/U8g2)** v2.34.22+ — OLED graphics library

## 🚀 Getting Started

### Prerequisites

- [PlatformIO IDE](https://platformio.org/install) (VS Code extension or CLI)
- ST-Link programmer/dongle
- Hardware components listed above

### Build & Upload

```bash
# Clone the repository
git clone https://github.com/wzgcavtc/PlatformIO.git
cd PlatformIO

# Build the project
pio run

# Upload to the board (via ST-Link)
pio run --target upload

# Monitor serial output (if applicable)
pio device monitor
```

### Project Structure

```
PlatformIO/
├── src/
│   └── main.cpp          # Main firmware source code
├── include/               # User header files (reserved)
├── lib/                   # Private libraries (reserved)
├── test/                  # Unit tests (reserved)
├── .vscode/               # VS Code configuration
├── platformio.ini         # PlatformIO project configuration
└── .gitignore             # Git ignore rules
```

## 🧠 How It Works

1. **Startup** — System initializes OLED display, sets up GPIO pins, configures stepper driver, and calculates target pulse frequency (200 RPM × 200 steps × 8 microsteps / 60 ≈ 5333 Hz).

2. **Light Detection Loop** — The LDR is polled every 50ms:
   - When **light is detected** (DO pin goes LOW): Motor starts running at 200 RPM in the current direction, LED turns on
   - When **light is lost** (DO pin goes HIGH): Motor stops immediately, LED turns off, direction flag toggles

3. **Direction Toggle** — Each time the motor stops due to light loss, the direction flag (`is_forward`) is toggled, so the next run will be in the opposite direction.

4. **OLED Display** — Shows real-time status:
   - **Running**: Forward/Reverse indicator, RPM, Hz, voltage, division ratio, current, LED status, mode
   - **Stopped**: "MOTOR STOPPED" message, LDR analog value, next direction

## 🔄 Future Improvements

- [ ] Add potentiometer for speed adjustment
- [ ] Implement acceleration/deceleration profiles
- [ ] Add serial command interface for remote control
- [ ] Support multiple motor operation modes (positioning, constant speed, etc.)
- [ ] Add EEPROM storage for configuration persistence
- [ ] Implement watchdog timer for fault recovery

## 📄 License

This project is open source. Feel free to use, modify, and distribute.

## 👤 Author

**wzgcavtc**

---

*Built with [PlatformIO](https://platformio.org/) | MCU: STM32F103C8 | Framework: Arduino*