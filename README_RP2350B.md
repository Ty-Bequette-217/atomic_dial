# Smart Knob RP2350B Port

This directory contains a port of the Smart Knob firmware from **ESP32** to **Raspberry Pi RP2350B**.

## Overview

The Smart Knob is a haptic output knob firmware project originally designed for the ESP32 microcontroller. This port adapts it to run on the RP2350B, the newest Raspberry Pi microcontroller with dual ARM Cortex-M33 cores.

### Key Changes from ESP32 Version

| Aspect | ESP32 | RP2350B |
|--------|-------|---------|
| **GPIO Numbering** | Arbitrary (0-39) | Sequential (GP0-GP29) |
| **PWM Controller** | LEDC (8-16 channels) | Programmable I/O (PIO) + Standard PWM (8 slices) |
| **Filesystem** | FFat (FAT32 on Flash) | LittleFS (traditional block storage) |
| **I2C/SPI** | Hardware I2C0, I2C1 / HSPI, VSPI | I2C0, I2C1 / SPI0, SPI1 |
| **LED Driver** | FastLED with RMT controller | FastLED with PIO-based WS2812 support |
| **UART** | Multiple options (CDC, UART) | UART0, UART1 |
| **FreeRTOS** | ESP-IDF FreeRTOS | Pico-SDK + mbed_rtos |

## Hardware Pin Mapping

All pin mappings have been defined in `platformio_rp2350b.ini`. Below is the RP2350B to ESP32 pin conversion:

### Motor Control (6PWM)
- **UH**: GPIO 7 (was GPIO 26)
- **UL**: GPIO 6 (was GPIO 25)
- **VH**: GPIO 3 (was GPIO 27)
- **VL**: GPIO 2 (was GPIO 32)
- **WH**: GPIO 5 (was GPIO 12)
- **WL**: GPIO 4 (was GPIO 33)

### Sensors
- **MT6701 Encoder** (SPI1):
  - CLK: GPIO 30 (was GPIO 13)
  - MISO: GPIO 28 (was GPIO 37)
  - CS: GPIO 29 (was GPIO 14)

- **TLV493D Magnetometer** (I2C0):
  - SDA: GPIO 12 (was GPIO 15)
  - SCL: GPIO 13 (was GPIO 8)

- **HX711 Strain Gauge**:
  - Data: GPIO 10 (was GPIO 38)
  - Clock: GPIO 11 (was GPIO 2)

### Display (SPI0)
- **MOSI**: GPIO 19 (was GPIO 5)
- **SCK**: GPIO 18 (was GPIO 20)
- **CS**: GPIO 17 (was GPIO 21)
- **DC**: GPIO 15 (was GPIO 22)
- **RST**: GPIO 20 (was GPIO 4)
- **BL**: GPIO 16 (was GPIO 19, PWM 0A)

### Other
- **LED Data (WS2812B)**: GPIO 27 (was GPIO 7)
- **TMC_DIAG**: GPIO 14 (was GPIO 36)
- **UART0** (Debug):
  - RX: GPIO 1
  - TX: GPIO 0
  
- **UART1** (USB Serial):
  - RX: GPIO 9
  - TX: GPIO 8

## Building

### Prerequisites

1. **PlatformIO** installed with Raspberry Pi support
2. **RP2350B board** support package installed
3. **Arduino-Pico framework** (earlephilhower's core)

### Build Command

```bash
# Build for RP2350B
platformio run -c platformio_rp2350b.ini -e view_rp2350b

# Clean and rebuild
platformio run -c platformio_rp2350b.ini -e view_rp2350b --target clean

# Upload to device
platformio run -c platformio_rp2350b.ini -e view_rp2350b --target upload
```

## Port Status

### Completed ✅
- [x] PlatformIO configuration
- [x] Core task framework (with FreeRTOS)
- [x] Motor task (BLDCDriver6PWM via SimpleFOC)
- [x] Configuration system (LittleFS instead of FFat)
- [x] Display task framework (TFT_eSPI)
- [x] Interface task (simplified UART-based)
- [x] Sensor driver stubs (MT6701, TLV493D)
- [x] Motor configurations (Wanzhida, MAD2804)
- [x] Protocol buffers (smartknob.pb.h/c)

### In Progress 🔄
- [ ] Display task GPU rendering
- [ ] Serial protocol implementation (plaintext + protobuf)
- [ ] Sensor driver full implementation
- [ ] LED driver (FastLED PIO integration)
- [ ] Strain gauge calibration

### TODO 📋
- [ ] Test motor calibration routine
- [ ] Verify PWM frequency on RP2350B
- [ ] Audio feedback implementation
- [ ] Performance optimization
- [ ] Full integration testing
- [ ] Bootloader setup

## Known Issues & Limitations

1. **Display Rendering**: Current implementation is minimal. Full sprite rendering not yet optimized for RP2350B.

2. **Serial Protocol**: Only basic UART logging works. Full plaintext and protobuf protocols need implementation.

3. **FastLED WS2812B**: Requires PIO-based implementation which adds complexity. Currently stubbed.

4. **I2C/SPI Timing**: May need fine-tuning for your specific hardware configuration.

5. **Memory**: RP2350B has less RAM (264KB vs 520KB on ESP32). Monitor heap usage during development.

## Architecture Notes

### FreeRTOS Tasks

Three main task cores are created:

1. **Motor Task** (Core 1): Runs the SimpleFOC control loop at ~1kHz
2. **Interface Task** (Core 0): Handles UART communication and logging
3. **Display Task** (Core 0): Renders to TFT display (if enabled)

### Pin Assignment Strategy

All GPIO assignments are handled via preprocessor defines in `platformio_rp2350b.ini`, making it easy to update pins by:
1. Modifying the `.ini` file
2. No source code changes required for pin reassignments

### Filesystem Differences

**ESP32 (FFat)**:
- Uses FAT32 filesystem on flash
- Familiar file API but less efficient

**RP2350B (LittleFS)**:
- Uses LittleFS (flash-optimized)
- Better for embedded systems
- File API is similar but slightly different

## Next Steps for Full Implementation

1. **Implement Serial Protocols**:
   - Review `firmware/src/serial/` in ESP32 version
   - Port plaintext and protobuf protocol handlers

2. **Complete Sensor Drivers**:
   - MT6701: Implement full SPI read with CRC
   - TLV493D: Implement I2C communication
   - HX711: Implement PIO-based or GPIO-based bit-banging

3. **Optimize Display**:
   - Port sprite rendering code from `display_task.cpp`
   - Adapt LED PWM control using RP2350B hardware

4. **Testing & Calibration**:
   - Run motor calibration routine
   - Verify detent feel and haptic feedback
   - Test sensor accuracy

## Debugging

### Enable Debug Logging

Set in `platformio_rp2350b.ini`:
```ini
-DCORE_DEBUG_LEVEL=ARDUHAL_LOG_LEVEL_DEBUG
```

### Monitor Serial Output

```bash
platformio device monitor -c platformio_rp2350b.ini
```

## License

This port maintains the same license as the original Smart Knob project.

## References

- [Smart Knob Original GitHub](https://github.com/scottbez1/smartknob)
- [RP2350B Datasheet](https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf)
- [Arduino-Pico Framework](https://github.com/earlephilhower/arduino-pico)
- [SimpleFOC Library](https://simplefoc.com)
- [TFT_eSPI Library](https://github.com/Bodmer/TFT_eSPI)
