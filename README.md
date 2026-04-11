![ESP32](https://img.shields.io/badge/platform-ESP32-blue)
![Status](https://img.shields.io/badge/status-in%20development-yellow)
![IoT](https://img.shields.io/badge/type-IoT-green)
![License](https://img.shields.io/badge/license-MIT-blue)
# 🔧 Hardware Test Bench – ESP32

A modular ESP32 firmware for testing electronic components during hardware prototyping and PCB validation. The system allows individual or full hardware testing through an interactive Serial Monitor menu.

## 📋 Features

* Interactive test menu
* Individual component testing
* Full system test option
* Serial Monitor feedback
* Modular test functions
* User confirmation validation

## 🧰 Tested Hardware

* Buzzer
* Relay
* Ultrasonic sensor
* Push button
* LED
* PIR motion sensor

## 📍 Pin Configuration

| Component          | GPIO |
| ------------------ | ---- |
| Buzzer             | 15   |
| Relay              | 5    |
| Ultrasonic Trigger | 19   |
| Ultrasonic Echo    | 18   |
| Button             | 21   |
| LED                | 22   |
| PIR Sensor         | 23   |

## ⚙️ How to Use

- 1 Upload the firmware to ESP32
- 2 Open Serial Monitor (**115200 baud**)
- 3 Select a test:

```
1 - Buzzer Test
2 - Relay Test
3 - Ultrasonic Test
4 - Button and LED Test
5 - PIR Test
6 - All Tests
```

- 4 Follow the instructions shown in the Serial Monitor

## 🚀 Future Improvements

* Automatic pass/fail detection
* Non-blocking tests
* OLED interface
* Test logs
* FreeRTOS tasks
* Hardware abstraction layer

## 👩‍💻 Author

**Maria Eduarda Pereira  de Jesus**

Computer Engineering Student | 
Embedded Systems | Hardware | IoT | 

---

