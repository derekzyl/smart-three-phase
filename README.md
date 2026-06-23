# Intelligent Three-Phase Load Protection System

An ESP32-based, IoT-enabled safety and monitoring device designed for industrial three-phase applications. The system provides real-time fault detection, local and remote monitoring, automated load isolation, and cellular-based SMS alerting to protect sensitive equipment against power supply anomalies.

---

## Project Overview

| Attribute | Details |
| :--- | :--- |
| **Project Code** | PG-PROTO-001 |
| **Project Type** | Postgraduate Research Prototype |
| **Domain** | Industrial Power Systems / Embedded Systems / IoT |
| **Platform** | ESP32 (Xtensa LX6 Dual-Core) + FreeRTOS |
| **Protocol Stack** | WiFi · MQTT · GSM (2G) · I2C · UART |
| **Development Environment** | PlatformIO · Flutter · Python 3.10+ |

---

### Background & Motivation

Three-phase power systems are the backbone of modern industrial and commercial electrical infrastructure, supplying power to motors, HVAC equipment, industrial machinery, and critical processing plant utilities across sub-Saharan Africa and beyond. Despite this centrality, power quality in many regions remains unreliable — characterized by frequent voltage fluctuations, phase loss events, and supply imbalances that are directly attributable to grid instability, aging distribution infrastructure, and uneven consumer load distribution.

The consequences of unprotected exposure to these faults are severe. **Phase loss** (the complete dropout of one supply phase) causes single-phasing in three-phase motors, generating excessive heat and mechanical stress that rapidly destroys motor windings. **Phase imbalance** — even when all three phases are nominally present — causes derating, vibration, and accelerated bearing failure. **Over-voltage spikes** damage insulation and capacitor banks, while **under-voltage conditions** lead to motor under-torque, stalling, and current draw, triggering thermal failure. In aggregate, these conditions account for a significant percentage of industrial equipment downtime, repair costs, and production losses.

Conventional protection relays exist in the market but are predominantly electromechanical or discrete-logic devices with limited fault-logging capability, no remote monitoring, and no cloud connectivity. A modern, microcontroller-driven solution with embedded intelligence, IoT connectivity, and cellular alerting represents a substantial upgrade to the state of practice — and forms the core rationale of this project.

---

### Problem Statement

> *How can a low-cost, embedded microcontroller-based protection system reliably detect and respond to three-phase power anomalies in real time while simultaneously enabling remote monitoring and alerting through both internet-based and cellular channels — and what constitutes a functional, academically-rigorous prototype of such a system?*

This project directly addresses that question through a hardware-software co-design approach, producing a fully functional prototype system as its primary artefact.

---

### Design Goals & Engineering Objectives

The system has been designed against the following prioritised engineering requirements:

1. **Real-Time Fault Detection:** The processing pipeline must identify all five target fault classes (over-current, under-voltage, over-voltage, phase loss, and phase imbalance) within a detection latency acceptable for industrial motor protection — targeting sub-100ms response from fault onset to relay trip.

2. **Automated Load Isolation:** Upon fault detection, the system must autonomously open the relay contacts, disconnecting the protected load without requiring any human intervention.

3. **Dual-Channel Alerting:** Fault events must be communicated to a human operator through two independent, parallel channels:
   * **Cellular SMS** via the embedded SIM800L GSM module — ensuring notification even when internet connectivity is absent.
   * **IoT Cloud Dashboard** via MQTT/Firebase — enabling timestamped fault log review, historical trend analysis, and real-time remote monitoring from any web browser or mobile device.

4. **Local Visual Interface:** An OLED display must present live per-phase voltage and current readings alongside the current protection status and most recent fault code, enabling on-site technicians to assess system state without requiring a mobile device or laptop.

5. **Extensibility & Maintainability:** The firmware architecture should be modular and clearly separated into functional tasks (sensing, protection logic, display, communications) to allow future enhancements such as additional sensor types, Modbus RTU support, or an on-device event log.

6. **Low-Cost, Accessible Implementation:** The prototype is constructed using commercially available development modules and veroboard assembly, without custom PCB fabrication — keeping the prototype cost tractable for an academic project whilst producing a functionally complete demonstration of the concept.

---

### System Architecture Summary

The system is structured as a three-tier embedded IoT architecture:

```
[ Three-Phase Mains (R/Y/B) ]
         │
    ┌────▼─────────────────────────┐
    │      Sensing Layer            │
    │  ZMPT101B × 3 (Voltage)      │
    │  ACS712-30A × 3 (Current)    │
    └────────────┬─────────────────┘
                 │ Analog ADC Signals
    ┌────────────▼──────────────────────────────────┐
    │             ESP32 Controller (Core)            │
    │  ┌──────────────────────────────────────────┐ │
    │  │  FreeRTOS Tasks                          │ │
    │  │  • ADC Sampling & RMS Calculation        │ │
    │  │  • Fault Detection & Threshold Logic     │ │
    │  │  • Relay Control (GPIO)                  │ │
    │  │  • OLED Rendering (I2C)                  │ │
    │  │  • MQTT / Firebase Telemetry (WiFi)      │ │
    │  │  • SIM800L AT Commands (UART)            │ │
    │  └──────────────────────────────────────────┘ │
    └──────────────┬──────────────┬─────────────────┘
                   │              │
     ┌─────────────▼──┐    ┌──────▼──────────────────┐
     │  Local Output   │    │    Remote Output         │
     │  • OLED Display │    │  • SMS Alert (SIM800L)   │
     │  • Relay Module │    │  • MQTT → Cloud Backend  │
     │    (Load Trip)  │    │  • Web / Mobile App      │
     └─────────────────┘    └──────────────────────────┘
```

The ESP32 acts as the central intelligence hub — it simultaneously manages real-time analog data acquisition, executes protection logic, drives the relay outputs, and handles two independent communication channels. A lightweight Python backend bridges MQTT topics to persistent cloud storage and serves a REST API to the Flutter-based cross-platform dashboard application.

---

### Key Deliverables

| # | Deliverable | Description |
| :-- | :--- | :--- |
| 1 | **Firmware (ESP32)** | C++ firmware built with PlatformIO; implements ADC sampling, RMS computation, all five fault detection algorithms, relay control, OLED interface, MQTT publishing, and SIM800L SMS dispatch. |
| 2 | **Backend Services** | Python application providing MQTT-to-database bridging, REST API endpoints for dashboard data, and historical event log management. |
| 3 | **IoT Dashboard** | Cross-platform Flutter web and mobile application showing live per-phase electrical readings, system status, active fault indicators, and historical alert logs. |
| 4 | **Panel Assembly** | Fully wired, DIN rail-mounted IP54 enclosure containing all modules, terminal blocks, cable management, and power supply infrastructure. |
| 5 | **Technical Documentation** | Circuit schematics, Bill of Materials, calibration procedures, test report (including fault injection validation), and system user guide. |

---

## Key Features

* **Advanced Protection & Fault Detection:**
  * **Over-Current Protection:** Automatically isolates the load when current exceeds set limits.
  * **Over-Voltage / Under-Voltage Monitoring:** Protects equipment from grid fluctuations.
  * **Phase Loss Detection:** Detects if any of the three input phases fail.
  * **Phase Imbalance Monitoring:** Calculates and triggers alerts for voltage/current asymmetry across phases.
* **Automated Isolation:** High-current relay modules (30A) act as circuit breakers, instantly disconnecting the load upon detecting any critical fault.
* **Localized OLED Interface:** High-contrast 1.3" display showing real-time root-mean-square (RMS) voltage, current, active status, and detailed fault diagnostics.
* **GSM Alert Mechanism:** SIM800L module triggers urgent SMS alerts to designated administrators with details of the fault.
* **Dual-Platform IoT Dashboard:** Real-time web and mobile dashboards display streaming sensor data and isolation history for remote diagnostics.

---

## Repository Structure

The project code is split into three main components:

* **[firmware/](file:///home/cybergenii/Desktop/codes/embedded/ai3phase/firmware)**: ESP32-based microcontroller firmware, written in C++ (PlatformIO). Coordinates sensor reading, fault detection algorithms, relay isolation, OLED rendering, SIM800L command sending, and WiFi MQTT client communication.
* **[backend/](file:///home/cybergenii/Desktop/codes/embedded/ai3phase/backend)**: Python services acting as a message broker bridge, processing telemetry streams, database persistence, and API services for remote dashboards.
* **[app/](file:///home/cybergenii/Desktop/codes/embedded/ai3phase/app)**: Cross-platform Flutter mobile and web application dashboard displaying live data, alert history, and manual override switches.

---

## Bill of Materials (BOM)

| Item | Component | Specification | Quantity | Purpose |
| :--- | :--- | :--- | :--- | :--- |
| **1** | ESP32 Development Board | 38-pin, 240MHz Dual-Core, WiFi + Bluetooth | 1 | Main protection controller & IoT client |
| **2** | ZMPT101B Voltage Sensor | Single-phase AC Voltage Module, 250V Max | 3 | Real-time voltage sensing per phase |
| **3** | ACS712 Current Sensor (30A) | Hall-effect Current Sensor, ±30A | 3 | Real-time current sensing per phase |
| **4** | 4-Channel Relay Module | 30A/250VAC per channel with optocoupler | 1 | Three-phase load isolation |
| **5** | SIM800L GSM Module | Quad-band 2G GSM/GPRS, UART interface | 1 | SMS-based emergency fault alerting |
| **6** | OLED Display | 1.3-inch, I2C, SH1106 controller (128x64) | 1 | Local display for live readings and diagnostics |
| **7** | LM2596 Buck Converter | Adjustable Input 7-35V to Output 3A Max | 2 | Voltage regulation for ESP32 and modules |
| **8** | AC-DC SMPS | 12V 2A Output, Enclosed | 1 | Primary system power supply |
| **9** | Control Panel Box | DIN Rail mountable, IP54 enclosure | 1 | Main physical housing |
| **10** | DIN Rail & Terminal Blocks | Assorted 20-position terminal rails | 1 | Clean and secure power distribution |
| **11** | Interconnects | Jumper wires, pin headers, terminal connectors | 1 | Component wiring |
| **12** | Hookup Wires & Consumables | 1.0mm hookup wire, solder, heat-shrink tubing | 1 | Overall assembly and connections |

---

## Hardware Architecture & Connections

### 1. Sensing Layer
* **Voltage Sensing:** Three **ZMPT101B** transformers step down the high-voltage AC phases (Red, Yellow, Blue) to low-voltage analog signals readable by the ESP32's Analog-to-Digital Converter (ADC).
* **Current Sensing:** Three **ACS712-30A** Hall-effect current sensors are connected in-series with each phase load line to measure instantaneous load current.

### 2. Processing & Logic (ESP32)
* Samples the analog signals at high frequencies to calculate Root-Mean-Square (RMS) voltages ($V_{RMS}$) and currents ($I_{RMS}$).
* Continuously checks values against preset safety thresholds:
  * **Under-voltage threshold:** e.g., $< 180V$ (nominal 220V phase-to-neutral).
  * **Over-voltage threshold:** e.g., $> 250V$.
  * **Over-current threshold:** Configuration-dependent.
  * **Phase imbalance:** Computes the deviation percentage between the three phases.
  * **Phase loss:** Instantly triggers isolation if $V_{RMS}$ of any phase drops to near zero while others are active.

### 3. Isolation & Alerts
* **Relay Output:** If any fault condition is met, the ESP32 immediately pulls the relay control pins to a safe state, de-energizing the 30A relay coils and disconnecting the load.
* **SMS Alert:** The ESP32 sends AT commands to the **SIM800L** module via a hardware serial interface to transmit a customized alert SMS to preset phone numbers.
* **IoT Dashboard Link:** Telemetry data is packetized into JSON format and published over MQTT (or directly to a Cloud database like Firebase) to update the web and mobile interfaces.

---

## Installation & Setup

### Requirements
* **Operating Voltage:** Three-phase AC mains ($380\text{V} - 415\text{V}$ line-to-line, $50\text{Hz}$).
* **Workspace:** DIN Rail enclosure wiring. Point-to-point connections on high-quality prototyping board (veroboard/stripboard) for the physical layout.
* **Development Environment:**
  * VS Code with PlatformIO extension (for ESP32 firmware compilation).
  * Flutter SDK (for dashboard mobile/web builds).
  * Python 3.10+ (for backend bridge tasks).

### Hardware Wiring Reference
1. Connect the mains inputs to the ZMPT101B input terminals (Ensure appropriate high-voltage precautions).
2. Wire the ACS712 current sensors in series with the phase lines before routing to the load terminal blocks.
3. Configure the LM2596 buck converters to output stable $5\text{V}$ and $3.3\text{V}$ lines for the ESP32, OLED, and SIM800L modules.
4. Route the control signal lines from the ESP32 GPIOs to the relay driver optocoupler inputs.
5. Connect the SIM800L UART interface ($TX/RX$) to the designated hardware serial pins on the ESP32.
6. Daisy-chain the SH1106 OLED display using the I2C bus ($SDA/SCL$) to the ESP32.

---

## Technical Exclusions & Scope Limitations

* **PCB Fabrication:** This version of the prototype is implemented using stripboard/veroboard point-to-point assembly.
* **Simulations:** Formal mathematical modeling (such as MATLAB/Simulink models) is excluded from the current hardware scope.
* **Mains Testing:** Testing requires three-phase grid access ($380\text{V}$, $50\text{Hz}$) and a test load at the testing site.
# smart-three-phase
# smart-three-phase
