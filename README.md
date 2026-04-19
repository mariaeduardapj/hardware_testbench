![ESP32](https://img.shields.io/badge/platform-ESP32-blue)
![Status](https://img.shields.io/badge/status-in%20development-yellow)
![IoT](https://img.shields.io/badge/type-IoT-green)
![Version](https://img.shields.io/badge/version-v1.2.1-blue)
![License](https://img.shields.io/badge/license-MIT-blue)

# 🔧 Hardware Test Bench – ESP32

A modular ESP32 firmware for testing electronic components during hardware prototyping and PCB validation.  
The system provides an interactive **web-based interface**, allowing users to trigger and monitor hardware tests directly from a browser.

---

## 🌐 New: Web Interface

The test system is now accessible via a local web server hosted on the ESP32.

- Access through the device's **local IP address**
- Control tests using **interactive buttons**
- View system version and status directly in the browser

---

## 📋 Features

### 🔹 Core Features
- Individual component testing
- Full system test execution
- Modular test structure
- User interaction and validation

### 🔹 New Features (v1.1.0 → v1.2.1)
- 🌐 Web interface for test control (replacing Serial menu)
- 🔊 Sound feedback when a test starts
- 🏷️ Version display in the interface
- 🛡️ Watchdog system (online/offline status monitoring)
- 🕒 Timestamped logging system for test execution

---

## 🧰 Tested Hardware

- Buzzer
- Relay
- Ultrasonic sensor
- Push button
- LED
- PIR motion sensor

---

## 📍 Pin Configuration

| Component          | GPIO |
|------------------|------|
| Buzzer           | 15   |
| Relay            | 5    |
| Ultrasonic Trigger | 19 |
| Ultrasonic Echo  | 18   |
| Button           | 21   |
| LED              | 22   |
| PIR Sensor       | 23   |

---

## ⚙️ How to Use

### 1. Upload firmware
Flash the firmware to your ESP32 using Arduino IDE or PlatformIO.

### 2. Connect to the device
- Connect ESP32 to your Wi-Fi network (or use AP mode if implemented)
- Find the device IP address via Serial Monitor

### 3. Access the Web Interface
- Open your browser
- Enter the ESP32 IP address

### 4. Run tests
- Use the on-screen buttons to trigger tests
- Monitor system behavior and logs in real time

---

## 🧾 Logging System

The system now records test execution with timestamps:
17:50:32 - Test started: PIR Sensor
17:50:59 - Test completed: PIR Sensor

This improves:
- Debugging
- Traceability
- Validation process

---

## 🛡️ System Monitoring

A watchdog mechanism tracks device status:

- ✅ Online → System operating normally  
- ❌ Offline → Possible failure or reset  

---

## 🚀 Future Improvements

- Remote dashboard (external access)
- OTA firmware updates
- Data export (logs download)
- Automatic pass/fail validation
- Non-blocking test execution
- FreeRTOS task management
- Hardware abstraction layer

---

## 👩‍💻 Author

**Maria Eduarda Pereira de Jesus**

Computer Engineering Student  
Embedded Systems | Hardware | IoT  

---