---
name: Esp32_Rfid
description: Proyecto ESP32 con módulo RC522 para lectura de tags NFC
type: project
---

## Overview
- **Board**: ESP32 DOIT DEVKit v1
- **Framework**: Arduino
- **Goal**: Aprender a leer tags NFC y posteriormente enviar señales a una aplicación

## Hardware
- **Módulo**: RC522 (RFID/NFC)
- **Conexión**: SPI

## Wiring RC522 → ESP32
- SDA (SS) → GPIO 5
- SCK → GPIO 18
- MOSI → GPIO 23
- MISO → GPIO 19
- GND → GND
- RST → GPIO 27
- 3.3V → 3.3V

## Plan de desarrollo
1. Leer tags NFC y mostrar UID por Serial
2. расширить para enviar señal a aplicación