# kvm-switch-controller

## Overview

This repo contains plans for a somewhat generic device that keeps multiple video switches in sync with a master switch.

The controlled video switches must be IR controllable (with a 3.5 mm IR receiver port). The device uses light sensors to monitor the active port on the master switch, and then plays back pre-captured fake "IR" signals to the controlled switches.

I am using this to optimize a dual-monitor Mac + PC KVM setup (multi-port KVM options are limited and no DisplayPort chaining support on Mac).

## Source Code Note

The code is for an ATmega328P (8-bit AVR microcontroller). As an experiment I implemented a basic cooperative thread scheduler, with a few thread-aware building blocks (mutexes, time, input).

Atmel Studio is required to build.

```
# Set to 8Mhz
avrdude -c usbtiny -p atmega328p -U lfuse:w:0xE2:m
# Flash & load eeprom backup
avrdude -c usbtiny -p atmega328p -U flash:w:kvm-controller.hex -U eeprom:w:eeprom.hex
```

## Schematic

```
[PC0] -- [1kΩ] -- [Port 1 LED]
[PC1] -- [1kΩ] -- [Port 2 LED]
[PC2] -- [1kΩ] -- [Status LED]
[PB1] -- [1kΩ] -- [IR in mirror LED]

[PD0] -- [Switch] -- Ground

         [IR  Vs] ----+--- [220Ω] -- Supply
[PB0] -- [IR Out]   [1uF]
         [IR Gnd] ----+--- Ground

                    [NPN C] --- [1kΩ] -- [Device 1 Supply] 
  [PB2] -- [1kΩ] -- [NPN B]   `--------- [Device 1 Sensor]
Port 1 out          [NPN E] -- Ground

                    [NPN C] --- [1kΩ] -- [Device 2 Supply] 
  [PB3] -- [1kΩ] -- [NPN B]   `--------- [Device 2 Sensor]
Port 2 out          [NPN E] -- Ground

[PD6/AIN0] -- [Sensor 1]
[PD7/AIN1] -- [Sensor 2]

NPN - 2N3904
IR Receiver - TSOP38238
Microcontroller - ATmega328P
```

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

## Compatibility

I don't suppose there is much standardization in the area of 3.5 mm IR ports, but in theory this can support a range of devices. IR capture timing resolution is roughly 0.25 ms.

I am using two of [these HDMI switches](https://www.newegg.com/p/2W4-00B9-00002). 
