
# Raspberry Pi PAC194x Multi-Rail Power Monitoring System

## Overview

This project implements a **multi-rail power monitoring system** using **Microchip PAC194x power monitor ICs** connected to a **Raspberry Pi over I²C**.

The system measures:

- **Bus Voltage (VBUS)**
- **Shunt Voltage (VSENSE)**
- **Load Current (I)**
- **Instantaneous Power (W)**

Measured data is streamed over **MQTT** for real-time telemetry visualization and remote control.

The architecture is designed for scalability and integration with dashboards, data logging systems, and IoT platforms.

---

## System Architecture

### Hardware Layer

- Raspberry Pi (**I²C master**)
- 3× PAC194x Power Monitor ICs
- External **1 Ω shunt resistors**

### Monitored Rails

- 3.3 V
- 1.2 V
- 5 V

Each PAC monitors a dedicated rail channel. Voltage and current are measured across precision shunt resistors and validated using multimeter measurements.

---

### Firmware Layer (C)

- Low-level I²C communication via Linux `i2c-dev`
- Direct register access:
  - `REFRESH`
  - `VBUSn`
  - `VSENSEN`
  - `VPOWERN`
- Big-endian multi-byte register reconstruction
- Raw-to-physical conversion implemented in software

### Scaling Formulas

```text
VBUS (V)   = 9.0 × (VBUS_raw / 65536)
VSENSE (V) = 0.1 × (VSENSE_raw / 65536)
I (A)      = VSENSE / Rsense
P (W)      = VBUS × I
