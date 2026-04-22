# The Atomic Dial
### by Yonathan Gur, Ryan Gelston, Ty Bequette, and Cody Fuh

---

## Overview
The Atomic Dial is an open-source, haptic-enabled smart knob powered by the RP2350B architecture. It provides software-defined mechanical feedback—such as detents, virtual end-stops, and magnetic resistance—using a brushless DC motor. An adaptation of the [SmartKnob](https://github.com/scottbez1/smartknob) project by Scott Bezek, this project adapts the original device to work smoothly with the RP2350B microcontroller. The board was designed to include header pins to plug in the [Proton development board](https://ece362-purdue.github.io/proton-labs/) designed by staff for ECE 36200: Microprocessor Systems and Interfacing at Purdue University.

## Board Design & Hardware Integration
Transitioning from the original ESP32-PICO architecture to the RP2350B required a redesign of the Base PCB and hardware routing to accommodate the Proton development board. We designed a custom baseplate PCB that features female headers specifically spaced to receive the Proton board. These header pins allow anyone with a Proton board to easily recreate this project and use it with their own boards. A bulk of the work done here was carefully rerouting the signals for each of the peripherals to match the correct function needed for compatability with the proton board (see [electronics/ProjectPinAssignments.xlsx](https://github.com/Ty-Bequette-217/atomic_dial/blob/main/electronics/ProjectPinAssignments.xlsx)).


## Firmware Architecture
To make the RP2350B drive a display, read an ADC, and commutate a motor simultaneously without stuttering, the firmware is divided into distinct, non-blocking layers built on the Raspberry Pi Pico C/C++ SDK.

### AS5600 Motor Encoder

### GC9A01 Round LCD

#### Initialization for GC9A01

#### Inclusion of LVGL C Library

### HX711 Motor Driver

### LDR monitoring for LCD Brightness

