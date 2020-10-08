# kvm-switch-controller

## Overview

This repo contains plans for a somewhat generic device that keeps multiple video switches in sync with a master switch.

I am using this to optimize my dual-monitor Mac + PC KVM setup. Mac doesn't support DisplayPort chaining, and dual-monitor KVM options are limited, so I built this.

The controlled video switches must be IR controllable (with a 3.5 mm IR receiver port). The device uses light sensors to monitor the active port on the master switch, and then plays back pre-captured fake "IR" signals to the controlled switches.

## Source Code Note

The code is for an ATmega328P (8-bit AVR microcontroller).

As an experiment I implemented a basic cooperative thread scheduler, with a few thread-aware building blocks (mutexes, time, input).

## Schematic

todo

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
