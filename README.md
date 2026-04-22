# The Atomic Dial

### An adaptation of the [SmartKnob](https://github.com/scottbez1/smartknob) project by Scott Bezek, this project adapts the original device to work smoothly with the RP2350B microcontroller. The board was designed to include header pins to plug in the Proton development board designed by staff for ECE 36200: Microprocessor Systems and Interfacing at Purdue University.

##### by Yonathan Gur, Ryan Gelston, Ty Bequette, and Cody Fuh

---

## ⚙️ Overview
The Atomic Dial is an open-source, haptic-enabled smart knob powered by the RP2350B architecture. It provides software-defined mechanical feedback—such as detents, virtual end-stops, and magnetic resistance—using a brushless DC motor. 

Beyond adapting the original SmartKnob to the Proton board, we expanded the interaction model by integrating a highly tuned Force Sensitive Resistor (FSR). This allows the dial to measure precise pressure inputs in addition to high-resolution rotary control.

## 🛠️ Board Design & Hardware Integration
Transitioning from the original ESP32-PICO architecture to the RP2350B required a redesign of the Base PCB and hardware routing to accommodate the Proton development board. We designed a custom baseplate PCB that features female headers specifically spaced to receive the Proton board. These header pins allow any ECE 362 student (or anyone with a Proton board for that matter) to easily recreate this project and use it with their own boards. A bulk of the work done here was carefully rerouting the signals for each of the peripherals to match the correct function needed for compatability with the proton board.

### The FSR Shunt Hack (Solving Hardware Incompatibility)
A major addition to this project is the pressure-sensitive screen. However, FSRs exhibit macroscopic resistance swings (up to 200,000 Ω), while the HX711 24-bit ADC is designed for microscopic load cell swings (max ~31 Ω at Gain 64). Connecting them directly saturates the amplifier.

To solve this, we implemented a **Parallel Shunt Configuration** on the Right Leg of our Wheatstone bridge:
* We placed a **100Ω Potentiometer in parallel with the FSR**.
* This forces the electricity through the path of least resistance, "squishing" the 200,000 Ω swing down to a precise 12.5 Ω micro-scale window.
* This allows the HX711 to read the FSR smoothly without hitting the digital ceiling (`±8388607`), while giving us a mechanical "sensitivity dial" to tune the pressure response.

## 💻 Software Architecture (The Code Layers)
To make the RP2350B drive a display, read an ADC, and commutate a motor simultaneously without stuttering, the firmware is divided into distinct, non-blocking layers built on the Raspberry Pi Pico C/C++ SDK.

### Layer 1: Hardware Abstraction & PIO (Low-Level)
* **PIO Display Driver:** Pushing pixels to a high-resolution circular display requires massive bandwidth. We offloaded the LCD communication protocol to the RP2350B's Programmable I/O (PIO) state machines. This frees up the main ARM Cortex cores.
* **PWM Motor Commutation:** Hardware PWM slices are configured to send precise sinusoidal waveforms to the TMC6300, controlling the magnetic fields of the brushless motor.

### Layer 2: Sensor Acquisition
* **Magnetic Encoder SPI:** Reads the absolute angular position of the knob at extremely high frequencies to know exactly where the user's hand is.
* **Custom HX711 Library:** A bit-banged GPIO library dynamically configurable for Gain 128, 64, or 32. It utilizes two's complement decoding and a software tare function to calibrate the FSR shunt bridge on boot.

### Layer 3: The Haptic Control Loop (Mid-Level)
* **PID & Torque Calculation:** This layer takes the current angular position (from the encoder) and calculates the required torque to simulate physical sensations. 
* If the user enters a "virtual detent," the math dictates a spike in opposing PWM duty cycle, telling Layer 1 to magnetically "snap" the motor into place.

### Layer 4: Application & UI (High-Level)
* The overarching state machine. It takes parsed rotary data and FSR pressure values to update the UI on the circular screen, allowing users to scroll through menus, click to select, and feel unique haptic profiles for different settings.

## 🚀 Building and Flashing
This project is built strictly using CMake and the Pico C SDK.

1. Clone this repository and configure your CMake environment.
   ```bash
   git clone [your-repo-link]
   cd atomic-dial
   mkdir build && cd build
   cmake ..
   make
