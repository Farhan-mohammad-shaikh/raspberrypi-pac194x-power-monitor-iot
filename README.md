# Raspberry Pi PAC194x Multi-Rail Power Monitoring System

## Overview

This project implements a scalable multi-rail power monitoring system using Microchip PAC194x power monitor ICs connected to a Raspberry Pi over I²C.

The system measures:

- Bus Voltage (VBUS)
- Shunt Voltage (VSENSE)
- Load Current (A)
- Instantaneous Power (W)

Measured data is streamed via MQTT for real-time telemetry visualization, logging, and IoT integration.

The architecture is modular and designed for scalability across multiple power rails and dashboard systems.

---

## System Architecture

### Hardware Layer

- Raspberry Pi (I²C master)
- 3× Microchip PAC194x power monitor ICs
- External 1 Ω precision shunt resistors
- Monitored rails:
  - 3.3 V
  - 1.2 V
  - 5 V

Each PAC194x device monitors a dedicated power rail. Voltage and current are measured across precision shunt resistors and validated using external multimeter measurements.

---

### Firmware Layer (C)

- Low-level I²C communication using Linux `i2c-dev`
- Direct register access:
  - REFRESH
  - VBUSn
  - VSENSEN
  - VPOWERN
- Big-endian multi-byte register reconstruction
- Raw-to-physical unit conversion implemented in software
- Structured JSON telemetry output

---

## Scaling Formulas

```
VBUS (V)   = 9.0 × (VBUS_raw / 65536)
VSENSE (V) = 0.1 × (VSENSE_raw / 65536)
I (A)      = VSENSE / Rsense
P (W)      = VBUS × I
```

Where:

- Full-scale VBUS = 9.0 V
- Full-scale VSENSE = 0.1 V
- Rsense = 1 Ω
- ADC denominator = 2¹⁶ (65536)

---

## Example JSON Output

```json
{
  "i2c_addr": "0x10",
  "channel": 1,
  "vbus_raw": 12345,
  "vsense_raw": 2345,
  "vpower_raw": 12345678,
  "vbus_V": 5.012376,
  "vsense_V": 0.003589,
  "current_A": 0.003589
}
```

Each measurement is printed as one JSON object per line, making it suitable for:

- MQTT publishing
- Subprocess piping
- Real-time dashboards (Node-RED, Grafana, etc.)

---

## MQTT Telemetry

Telemetry is published using structured topics:

```
<i2c_addr>/ch<channel>
```

Examples:

```
0x10/ch1
0x11/ch1
0x12/ch3
```

This allows automatic separation of multiple rails in dashboard charts.

---

## Build & Run

### Enable I²C on Raspberry Pi

```
sudo raspi-config
```

Navigate to:

```
Interface Options → I2C → Enable
```

---

### Compile

```
make
```

---

### Run

```
sudo ./main
or
make run
```

---

## Key Features

- Multi-rail real-time power monitoring
- Direct low-level register control of PAC194x
- Clean JSON telemetry output
- MQTT-ready data streaming
- Scalable architecture for IoT systems
- Dashboard integration support

---

## Applications

- Embedded system power profiling
- IoT energy monitoring
- Multi-rail board validation
- Real-time system diagnostics
- Research and development measurement platforms

---

## Future Improvements

- Support for additional PAC channels
- Energy accumulation support
- Configurable sampling rates
- Automatic device detection
- Integration with cloud analytics platforms


---

## Author

Farhan Mohammad Shaikh  
Master’s Student – Microelectronics & Embedded Systems  
Embedded Systems | IoT | Low-Level Linux Development
