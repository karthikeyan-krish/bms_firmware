# pigweed_play 🎛️

[![Build with Bazel](https://img.shields.io/badge/build-Bazel-green)](https://bazel.build)
![FreeRTOS](https://img.shields.io/badge/RTOS-FreeRTOS-blue)
![Pigweed](https://img.shields.io/badge/framework-Pigweed-purple)
![C++17](https://img.shields.io/badge/language-C++17-orange)
![License](https://img.shields.io/badge/license-MIT-lightgrey)

A modular embedded firmware project demonstrating a **Battery Management System (BMS) software architecture** using modern C++, FreeRTOS, and Pigweed.

---

## 🚀 Overview

`bms_firmware` is an embedded systems project designed to showcase:

- Modular **BMS software architecture**
- **Real-time task separation** (acquisition, processing, communication)
- Event-driven design using **Active Objects (AO)**
- **Finite State Machine (FSM)** for system control and safety handling
- Sensor simulation with **fault injection**
- **Filtering, hysteresis, and debounce-based safety logic**
- Structured logging with **Pigweed**
- Unit testing with **Google Test**
- **Dual-slot bootloader with rollback support**

---

## 🎯 Objectives

This project focuses on building a realistic embedded system that demonstrates:

- Clean separation between **drivers, services, and application logic**
- Robust handling of **sensor data and fault conditions**
- Deterministic behavior under **real-time constraints**
- Scalable architecture suitable for **automotive-like systems (BMS)**

---


## ✨ Features

### 🧩 BMS Core
- Simulated sensors:
  - Voltage (mV)
  - Current (mA)
  - Temperature (deci °C)
- Sensor fault injection:
  - spike
  - noise
  - stuck values
  - disconnection
- Multi-threaded architecture:
  - **Acquisition thread**
  - **Processing thread**
  - **Communication thread**

---

### 🧠 Signal Processing & Safety
- Integer-based filtering (no floating point)
- Exponential moving average (low-pass filter)
- Optional median filtering for noisy signals
- Safety mechanisms:
  - threshold monitoring
  - hysteresis
  - debounce timing
- Fault detection:
  - overvoltage / undervoltage
  - overcurrent
  - overtemperature
  - sensor faults

---

### 🔄 BMS State Machine
- INIT → system initialization & checks
- IDLE → safe standby state
- DRIVE → normal operation
- SAFETY states:
  - voltage fault
  - current fault
  - temperature fault

---

### 📡 Communication
- Command-based interface (UART or simulated)
- Supported commands:
  - `GET_VALUES`
  - `SET_VALUES`
  - `GET_STATE`
  - `GET_FAULTS`
  - `SET_MODE IDLE`
  - `SET_MODE DRIVE`

---

### ⚙️ Bootloader (A/B + Rollback)
- Dual-slot firmware update mechanism
- Safe update with inactive slot flashing
- Metadata-driven control:
  - active / confirmed / pending slots
  - version, size, CRC
- Automatic rollback:
  - invalid firmware detection
  - boot failure recovery

---

### 🧵 Concurrency Model
- Built on **FreeRTOS via Pigweed**
- Event-driven architecture using Active Objects
- Thread-safe communication via queues
- ISR decoupling through work queue

---

### 🧪 Testing
- Unit tests using **Google Test**
- Hardware abstraction via fake drivers
- Coverage includes:
  - filtering logic
  - safety thresholds
  - state machine transitions
  - bootloader logic

---

## 📖 Design Highlights

### 🟢 Architecture
- Clear separation of concerns:
  - **Drivers** → hardware / simulation
  - **Domain** → BMS logic
  - **Threads** → scheduling & execution
- Modular and scalable design

---

### 🔵 Embedded Constraints
- Integer-based signal processing (no floating point)
- Deterministic execution
- Low memory footprint
- Real-time safe behavior

---

### 🟡 Safety Strategy
Raw Sensor
- Validation (range / plausibility)
- Filtering
- Threshold check
- Hysteresis
- Debounce
- Fault decision
- State transition

---

## ♻️ Reusable Architecture Foundation

Parts of this project (Active Objects framework, threading model, and bootloader) are based on an embedded systems blueprint ([pigweed_play](https://github.com/karthikeyan-krish/pigweed_play.git)) developed by me.

This allowed focusing on higher-level system design such as BMS logic, safety handling, and multi-threaded data flow, while reusing a proven and tested architectural foundation.

---

## ⚡ Build & Run
```bash
# Clone project
git clone https://github.com/karthikeyan-krish/bms_firmware.git
cd bms_firmware

# Build with Bazel
bazel build //apps:application.elf --platforms=//targets/stm32l4xx:platform

# Flash to board
bazel run //tools:flash_application --platforms=//targets/stm32l4xx:platform
