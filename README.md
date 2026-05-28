# STM32 Motor Control System

Embedded motor control system based on the STM32 platform featuring a graphical TFT user interface, multiple operating modes, and menu-based system control.

All peripheral drivers used in this project were developed from scratch using direct register-level programming without relying on STM32 HAL libraries.

---

## Project Overview

This project demonstrates low-level embedded software development, modular system architecture, and hardware-oriented control logic by combining motor control, user interface design, and peripheral driver development.

The system is designed around a stepper motor application and can be extended for additional actuator or automation use cases.

---

## Key Features

- Stepper motor control with configurable movement profiles
- Operating modes: ECO / NORMAL / FAST
- TFT graphical menu interface
- Rotary encoder menu navigation
- Status monitoring screen
- Direction control
- Start / Stop motor commands
- Expandable role-based access control using RFID (in progress)

---

## Low-Level Driver Development

All hardware interfaces were implemented manually via STM32 register access:

- GPIO control
- SPI communication
- Timer / delay functions
- Motor output handling
- Display communication
- User input processing


---

## Software Architecture

```text

main.c
├── motor_control.c  -> Motor logic / modes / state handling
├── stepper.c        -> Stepper motor sequencing / movement control
├── ui.c             -> TFT menu system / navigation
├── tft.c            -> ST7735 display driver
├── font.c           -> Font rendering for text output
├── spi.c            -> Register-level SPI communication
└── timer.c          -> Delay / timing functions
```

## User Interface Structure
Main Menu
 ├── Start
 ├── Stop
 ├── Status
 ├── MODE

Mode Menu
 ├── ECO
 ├── NORMAL
 ├── FAST
 ├── Back

Status Menu
 ├── Current Mode
 ├── Motor State
 ├── Direction
 ├── Back

## Hardware Prototype

![Motor Control System](motor_control_setup.jpeg)

Author

Morel Tonfack

GitHub: https://github.com/FranckMorel
