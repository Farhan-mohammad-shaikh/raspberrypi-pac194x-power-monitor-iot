# raspberrypi-pac194x-power-monitor-iot
Multi-channel power monitoring system using PAC194x senors and Raspberry pi 5. Features real-time voltage, current,  and power measurement over I2C, MQTT telemetry streaming, and remote command control with planned GUI visualization 
Overview

This project implements a multi-rail power monitoring system using Microchip PAC194x power monitor ICs connected to a Raspberry Pi over I²C.

The system measures:

Bus Voltage (VBUS)

Shunt Voltage (VSENSE)

Load Current (I)

Instantaneous Power (W)

Measured data is streamed over MQTT for real-time telemetry visualization and remote control. The architecture is designed for scalability, enabling integration with dashboards, data logging systems, and IoT platforms.

System Architecture
Hardware Layer

Raspberry Pi (I²C master)

3× PAC194x Power Monitor ICs

External 1 Ω shunt resistors

Monitored rails:

3.3 V

1.2 V

5 V

Each PAC monitors a dedicated rail channel. Voltage and current are measured across precision shunt resistors and validated using multimeter measurements.

Firmware Layer (C)

Low-level I²C communication via Linux i2c-dev

Direct register access:

REFRESH

VBUSn

VSENSEN

VPOWERN

Big-endian register reconstruction

Raw-to-physical conversion implemented in software

Scaling formulas:

VBUS (V)   = 9.0 × (VBUS_raw / 65536)
VSENSE (V) = 0.1 × (VSENSE_raw / 65536)
I (A)      = VSENSE / Rsense
P (W)      = VBUS × I


Rsense = 1 Ω

Communication Layer (MQTT)

Mosquitto broker

Telemetry publishing via MQTT topics

JSON payload format

Command subscription for remote control (LED control, sampling rate adjustment)

Example topic structure:

lab/pac/<address>/ch<n>/telemetry
lab/pi/cmd/led


Example payload:

{
  "addr": "0x10",
  "channel": 1,
  "vbus": 3.315,
  "current": 0.003186,
  "power": 0.01056
}

Features

Multi-device I²C communication

Channel-specific rail monitoring

Verified voltage/current measurements against hardware test points

Real-time telemetry streaming

Remote command handling via MQTT

Modular structure for GUI integration

Hardware Validation

Measurements were validated using:

Test points (SHUNT+ / SHUNT−)

Multimeter voltage verification

Ohm’s law cross-checks

Example validation:

VSENSE = 3.186 mV

Rsense = 1 Ω

Current ≈ 3.186 mA

Project Structure
raspberrypi-pac194x-power-monitor/
│
├── src/
│   ├── main.c
│   ├── pac194x.c
│   └── pac194x.h
│
├── docs/
│   ├── register-map.md
│   ├── measurement-results.md
│   └── system-architecture.md
│
├── Makefile
└── README.md

Build Instructions

Compile:

make


Run:

make run
