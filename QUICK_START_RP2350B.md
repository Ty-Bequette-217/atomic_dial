# Quick Start Guide - RP2350B Port

## 1. Setup

```bash
# Navigate to project directory
cd c:\Users\cocou\362_smart_knob

# Install PlatformIO Raspberry Pi support if needed
python -m pip install platformio
pio platform install raspberrypi
```

## 2. Build

```bash
# Use the RP2350B-specific configuration
pio run -c platformio_rp2350b.ini -e view_rp2350b

# Or with verbose output for debugging
pio run -c platformio_rp2350b.ini -e view_rp2350b -v
```

## 3. Upload

```bash
# Upload to connected RP2350B in bootloader mode
pio run -c platformio_rp2350b.ini -e view_rp2350b --target upload
```

##  4. Monitor

```bash
# Open serial monitor at 921600 baud
pio device monitor -c platformio_rp2350b.ini --speed 921600
```

## Directory Structure

```
firmware_rp2350b/
├── src/
│   ├── main.cpp                    # Entry point
│   ├── configuration.h/cpp         # Config management (LittleFS)
│   ├── motor_task.h/cpp            # Motor control loop
│   ├── display_task.h/cpp          # Display rendering
│   ├── interface_task.h/cpp        # UART interface
│   ├── task.h                      # FreeRTOS task base
│   ├── logger.h                    # Logging interface
│   ├── semaphore_guard.h           # RAII for semaphores
│   ├── util.h/cpp                  # Utility functions
│   ├── interface_callbacks.h       # Protocol callbacks
│   ├── mt6701_sensor.h/cpp         # MT6701 encoder
│   ├── tlv_sensor.h/cpp            # TLV493D magnetometer
│   ├── maq430_sensor.h             # MAQ430 placeholder
│   ├── motors/
│   │   ├── motor_config.h          # Motor config selector
│   │   ├── wanzhida_once_top.h     # Wanzhida motor params
│   │   └── mad2804.h               # MAD2804 motor params
│   ├── proto_gen/
│   │   ├── smartknob.pb.h          # Protobuf definitions
│   │   └── smartknob.pb.c          # Protobuf implementation
│   └── serial/
│       ├── uart_stream.h           # UART interface
│       ├── serial_protocol_plaintext.h  # Plaintext protocol
│       └── serial_protocol_protobuf.h   # Protobuf protocol
├── lib/                            # External libraries go here
├── include/                        # Header includes
└── test/                          # Unit tests (future)
```

## Device Pin Layout (RP2350B)

**Motor Control (6-PWM):**
```
GPIO 7   → UH (Phase U High-side)  
GPIO 6   → UL (Phase U Low-side)   
GPIO 3   → VH (Phase V High-side)  
GPIO 2   → VL (Phase V Low-side)   
GPIO 5   → WH (Phase W High-side)  
GPIO 4   → WL (Phase W Low-side)   
```

** Display (SPI0):**
```
GPIO 19  → MOSI (Serial Data)
GPIO 18  → SCK  (Serial Clock)
GPIO 17  → CS   (Chip Select)
GPIO 15  → DC   (Data/Command)
GPIO 20  → RST  (Reset)
GPIO 16  → BL   (Backlight PWM)
```

**Encoder (SPI1):**
```
GPIO 30  → CLK  (Serial Clock)
GPIO 28  → MISO (Serial Data In)
GPIO 29  → CS   (Chip Select)
```

**Sensors (I2C0 & Others):**
```
GPIO 12  → SDA  (I2C Data) - TLV493D, VEML7700
GPIO 13  → SCL  (I2C Clock)
GPIO 10  → HX711 Data
GPIO 11  → HX711 Clock
GPIO 27  → WS2812B LED Data
GPIO 14  → TMC Driver Diag
```

## Troubleshooting

### Compilation Errors

**Error: `BLDCDriver6PWM not found`**
- SimpleFOC library not installed
- Solution: Will be auto-installed by PlatformIO

**Error: `LittleFS not available`**
- Arduino-Pico core version issue
- Solution: Update to latest earlephilhower core

### Runtime Issues

**Motor not moving:**
1. Check pin mappings in `platformio_rp2350b.ini`
2. Verify PWM signal with oscilloscope
3. Check BMXxxxx driver power supply (5V)
4. Run calibration command via UART ('c' key)

**Display stays blank:**
1. Verify SPI pins are correct
2. Check TFT backlight PWM setup
3. Check TFT reset pin (GPIO 20)

**I2C sensor not responding:**
1. Verify GPIO 12/13 are accessible
2. Add pull-up resistors (4.7kΩ typical)
3. Check sensor power supply (usually 3.3V)

## Next Steps

See `README_RP2350B.md` in project root for:
- Full architecture overview
- Port status and roadmap
- Known limitations
- References and resources
