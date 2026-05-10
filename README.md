![ESP32](https://img.shields.io/badge/platform-ESP32-blue)
![Status](https://img.shields.io/badge/status-active-green)
![IoT](https://img.shields.io/badge/type-IoT-green)
![Version](https://img.shields.io/badge/version-v1.4.0-blue)
![License](https://img.shields.io/badge/license-MIT-blue)

# 🔧 Hardware Test Bench – ESP32

A modular ESP32-based system for testing electronic components during hardware prototyping and PCB validation.

The project evolved from a simple serial-based tester into a **web-based, configurable hardware validation platform**, designed to simplify testing workflows and improve traceability.

---

## 🌐 Web Interface

The system is fully accessible through a browser.

- Control tests via interactive UI
- Monitor real-time status
- View logs and results
- Configure custom devices

---

## 🚀 Key Features

### 🔹 Core System
- Individual component testing
- Full system test execution
- Modular architecture
- Real-time feedback

---

### 🔹 Advanced Features

#### 🧩 Custom Test Builder
Create and configure tests dynamically via the web interface:
- Select device type (e.g., PIR, LED)
- Assign GPIO pins
- Run tests without modifying firmware

---

#### ✅ Hybrid PASS/FAIL System
- Automatic validation based on expected behavior
- Suggested result (PASS/FAIL)
- Manual confirmation and override
- Improves reliability and flexibility

---

#### 🕒 Timestamped Logging System
- Tracks all test executions
- Includes:
  - start time
  - end time
  - result
  - messages

---

#### 📥 Log Export
- Download logs directly from the interface
- Supported formats:
  - `.txt`
  - `.csv`
- Enables external analysis and reporting

---

#### 📡 Wi-Fi Modes (AP + STA)
Simplified user setup:

- **Access Point (AP mode)**  
  - ESP32 creates its own Wi-Fi network  
  - Direct access via `192.168.4.1`

- **Station Mode (STA)**  
  - Connects to user’s Wi-Fi network  
  - Enables integration with local infrastructure  

- **Provisioning system**:
  - First-time setup via browser
  - No code modification required

---

## 🧰 Tested Hardware

- Buzzer
- Relay
- Ultrasonic sensor
- Push button
- LED
- PIR motion sensor

---

## ⚙️ How to Use

### 🔹 First Use (No Configuration Required)

1. Power on the ESP32  
2. Connect to Wi-Fi:  
   `TestBench_ESP32`  
3. Open browser:  
   `http://192.168.4.1`  
4. Configure your Wi-Fi network  

---

### 🔹 Normal Use

1. Access the system via browser  
2. Add or configure devices  
3. Run tests  
4. Monitor results and logs  

---

## 🧾 Logging Example
17:50:32 - Test started: PIR Sensor
17:50:59 - Test completed: PIR Sensor (PASS)

---

## 🛡️ System Monitoring

- Watchdog-based status tracking
- Detects:
  - system failure
  - connectivity issues

---

## 🎯 Project Status

This project is now functionally complete as a **hardware validation platform**.

Future updates will focus on:
- adding new device tests
- improving robustness
- expanding real-world usage

---

## 👩‍💻 Author

**Maria Eduarda Pereira de Jesus**

Computer Engineering Student  
Embedded Systems | Hardware | IoT  

---