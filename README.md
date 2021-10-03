# led-ir-relay

## Overview

This repo contains plans for a device that replays pre-captured IR sequences depending on which of 2 light sensors is receiving more light.

I am using this to keep two separate HDMI switches in sync with a KVM switch. I.e. monitor the LEDs on the KVM switch and replay IR to the HDMI switches.

## Source Code Notes

The code is for an ATmega328P (8-bit AVR microcontroller). Compile with avr-gcc (see Makefile) or AVR Studio.

The control flow is unusual. For the sake of novelty, I implemented a basic cooperative thread scheduler, with a few thread-aware apis (mutexes, timers, input).

```
# Set to 8Mhz
avrdude -c usbtiny -p atmega328p -U lfuse:w:0xE2:m
# Flash & load eeprom backup
avrdude -c usbtiny -p atmega328p -U flash:w:kvm-controller.hex -U eeprom:w:eeprom.hex

# Preserve EEPROM during chip erase (optional):
avrdude -c usbtiny -p atmega328p -U hfuse:w:0xD1:m
# 5V brown-out detection (optional):
avrdude -c usbtiny -p atmega328p -U efuse:w:0xFB:m
```

## Schematic

```
[PC0] -- [1kΩ] -- [Port 1 Status LED]
[PC1] -- [1kΩ] -- [Port 2 Status LED]
[PC2] -- [1kΩ] -- [Status LED]
[PB1] -- [1kΩ] -- [IR in mirror LED]

[PD0] -- [Switch] -- Ground

         [IR  Vs] ----+--- [220Ω] -- Supply
[PB0] -- [IR Out]   [1uF]
         [IR Gnd] ----+--- Ground

  [PB2] -- [1kΩ] -- [Port 1 IR LED Out]
  [PB3] -- [1kΩ] -- [Port 2 IR LED Out]

[PD6/AIN0] -- [Sensor 1 in]
[PD7/AIN1] -- [Sensor 2 in]

```

Specific parts used:
- IR Receiver- TSOP38238
- Microcontroller- ATmega328P
- 2x [LilyPad light sensor](https://www.sparkfun.com/products/14629)

## UI Reference

Single button UI.

Root menu:
- Short press to switch modes (solid: auto-detect, blinking: force port selection)
- Long press to enter IR capture menu
    - Short press to select bank or return to root (1 or 2 pulses: device 1 capture for port 1 or 2 respectively, 3 or 4 pulses: device 2 capture for port 1 or 2 respectively)
    - Long press to begin recording into selected bank
        - Short press to cancel
        - Long press to save
            - Rapid blink means error saving (0 length or overflow)
            - Status led temporary off means success saving
