# RP2350B Smart Knob Port - Common Issues & Solutions

## Compilation Issues

### Issue: "Cannot find SimpleFOC"
**Cause**: Library not in LittleFS search path
**Solution**: 
```bash
cd firmware_rp2350b/lib
git clone https://github.com/simplefoc/Arduino-FOC.git
```

### Issue: "Configuration.h: expected ';' before 'LittleFSGuard'"
**Cause**: Header guard or include issue with LittleFS
**Solution**: Verify `#include <LittleFS.h>` is before class definition

### Issue: "Undefined reference to BLDCDriver6PWM"
**Cause**: SimpleFOC not properly linked
**Solution**: Ensure `lib_deps = askuric/Simple FOC @ 2.2.0` in platformio_rp2350b.ini

### Issue: "GPIO constants not defined"
**Cause**: Missing -DPIN_* defines from .ini file
**Solution**: Check platformio_rp2350b.ini has all PIN_* definitions

---

## Runtime Issues

### Motor Not Spinning

1. **Check PWM output**:
   ```bash
   # Use oscilloscope on GPIO 7 (UH pin)
   # Should see ~20kHz PWM when motor_task runs
   ```

2. **Check motor connections**:
   - Verify 6 cables to motor controller (UH, UL, VH, VL, WH, WL)
   - Check power supply (5V to driver)

3. **Calibration needed**:
   - Send 'c' via UART to start calibration
   - Motor should spin both directions during calibration

4. **Check sensor**:
   ```cpp
   // In motor_task.cpp, verify encoder is initialized:
   encoder.getSensorAngle();  // Should return 0-6.28 radians
   ```

### Display Blank/Wrong Output

1. **TFT not initialized**:
   ```bash
   # Check pins GPIO 15-20 are accessible
   # Try `tft_.fillScreen(TFT_RED)` to fill with color
   ```

2. **SPI speed too fast**:
   - Reduce `SPI_FREQUENCY` in .ini from 40MHz to 10MHz
   - Try: `-DSPI_FREQUENCY=10000000`

3. **Reset pin issue**:
   - GPIO 20 (RST) must be pulled HIGH after reset
   - Check hardware for RC debounce on reset line

### I2C Sensors Not Found

1. **Check pull-ups**:
   - GPIO 12 (SDA) and GPIO 13 (SCL) need 4.7kΩ pull-ups to 3.3V
   - Test with multimeter: should see 3.3V when pulled

2. **Check power**:
   - Magnetometer (TLV493D) needs 3.3V
   - Some boards have 5V, will fry sensor - check carefully!

3. **Verify I2C address**:
   ```cpp
   // Common TLV493D addresses:
   0x5E  // Default
   0x1F  // Alt address
   ```

4. **Slow I2C speed**:
   - Try 100kHz instead of 400kHz:
   ```cpp
   Wire.setClock(100000); // Instead of 400000
   ```

### HX711 Not Responding

1. **Check GPIO 10/11**:
   - GPIO 10: Data (DOUT)
   - GPIO 11: Clock (SCK)
   - Verify with multimeter

2. **Power supply for strain gauge**:
   - Usually 5V
   - Check with multimeter

3. **Remove and reseat connections**:
   - Strain gauge can be picky about connections

### Serial Monitor Gibberish

1. **Wrong baud rate**:
   - Should be 921600
   - Not 115200 or 9600!

2. **USB cable issue**:
   - Try different USB cable/port
   - Some cables are charging-only (no data)

3. **Device not recognized**:
   - Put board in BOOTSEL mode
   - May need CP2102/CH340 driver

---

## Performance Issues

### Motor Jerky/Unstable

1. **Loop frequency too low**:
   - Increase task priority in motor_task.h
   - Check if other tasks are blocking

2. **PID tuning**:
   - Edit motor config file:
   ```cpp
   // motors/wanzhida_once_top.h
   #define FOC_PID_P 4.0   // Increase for snappier
   #define FOC_PID_D 0.04  // Small increment for damping
   #define FOC_PID_I 0     // Usually keep at 0
   ```

3. **Voltage limit**:
   ```cpp
   #define FOC_VOLTAGE_LIMIT 5  // Reduce if motor overheating
   ```

### LEDs Flickering

1. **PIO underutilization**:
   - FastLED PIO implementation needs verification
   - May need to disable other SPI/I2C during LED update

2. **Timing critical**:
   - WS2812B requires precise timing (~1.25µs per bit)
   - Use dedicated PIO state machine

### Serial Lag

1. **Baud rate mismatch**:
   - Should match MONITOR_SPEED (921600)

2. **Buffer overflow**:
   - Check `xQueueCreate` sizes in interface_task.h
   - May need larger queue if TX-heavy

---

## Hardware Diagnostics

### Can't Put Device in BOOTSEL Mode

1. **Try double-clicking BOOTSEL button**:
   - First click at power-up
   - Quickly click again

2. **Hold while plugging in USB**:
   - Insert USB while holding BOOTSEL
   - LED should turn off (enter bootloader)

3. **Try using `bossac` tool**:
   ```bash
   pio platform install raspberrypi --with-package=tool-bossac
   ```

### Weird GPIO Behavior

1. **Check for shorts**:
   - Use oscilloscope/multimeter for continuity
   - Check for cold solder joints

2. **Verify GPIO not claimed**:
   - Some GPIOs have special functions (QSPI, etc.)
   - Avoid GPIO 26-29 (QSPI pins)

3. **Power delivery**:
   - USB power may be insufficient for full system
   - Add external 5V supply if devices don't initialize

---

## Debug Techniques

### Add Debug Output

```cpp
// In any source file:
#include <Arduino.h>

void logDebug(const char* msg) {
    Serial.print("[DEBUG] ");
    Serial.println(msg);
}

// Call from anywhere:
logDebug("Motor initialized");
```

### Monitor Free Memory

```cpp
void checkMemory() {
    Serial.print("Free heap: ");
    Serial.print(rp2040.getFreeHeap());
    Serial.println(" bytes");
}
```

### Check Task Stack Usage

```cpp
// In FreeRTOS:
UBaseType_t highWater = uxTaskGetStackHighWaterMark(NULL);
Serial.print("Stack used: ");
Serial.println(highWater);
```

### Oscilloscope Signals to Check

| Pin | Expected Signal | Frequency |
|-----|-----------------|-----------|
| GPIO 7-6 (UH, UL) | Complementary PWM | ~20kHz |
| GPIO 30 (MT CLK) | SPI Clock | ~1-5MHz |
| GPIO 12/13 (I2C) | I2C waveform | ~100-400kHz |
| GPIO 27 (LED) | WS2812 timing | ~800kHz |
| GPIO 1 (RX) | UART RX data | 921600 baud |

---

## Known Workarounds

### Poor I2C Reliability
- Add 100nF capacitors near sensor VCC
- Add 10Ω series resistors on SDA/SCL
- Reduce I2C clock speed to 100kHz

### Display Ghosting
- Reduce SPI speed from 40MHz to 20MHz
- Add 1µF capacitor near CS pin

### Motor Cogging Resistance
- Increase FOC_PID_P slightly
- Check encoder CRC (MT6701)
- Verify motor controller voltages

---

## When All Else Fails

1. **Flash firmware again**:
   ```bash
   pio run -c platformio_rp2350b.ini -e view_rp2350b --target clean
   pio run -c platformio_rp2350b.ini -e view_rp2350b --target upload
   ```

2. **Check with minimal firmware**:
   - Create test_blink.cpp that just blinks an LED
   - Verify basic GPIO works before full firmware

3. **Update libraries**:
   ```bash
   pio lib update
   pio platform update
   ```

4. **Verify bootloader**:
   ```bash
   # Check if board appears in device manager
   # If not, may need to reflash bootloader
   ```

5. **Post on GitHub Issues**:
   - Smart Knob: https://github.com/scottbez1/smartknob/issues
   - Arduino-Pico: https://github.com/earlephilhower/arduino-pico/issues
   - Include: error message, board model, pin config

---

## Quick Reference: GPIO Functions

```
GPIO 0-1    UART0 RX/TX (Debug)
GPIO 2-5    PWM 1A,B + PWM 2A,B (Motor VL,VH + WL,WH)
GPIO 6-7    PWM 3A,B (Motor UL,UH)
GPIO 8-9    UART1 TX/RX (USB Serial)
GPIO 10-11  Strain gauge data/clock
GPIO 12-13  I2C0 SDA/SCL (Sensors)
GPIO 14     TMC Diag input
GPIO 15     LCD_DC (Data/Command)
GPIO 16     LCD_BL (Backlight PWM 0A)
GPIO 17     LCD_CS (SPI0 CS)
GPIO 18     LCD_SCK (SPI0 Clock)
GPIO 19     LCD_MOSI (SPI0 Data)
GPIO 20     LCD_RST (Reset)
GPIO 27     LED_DATA (WS2812B)
GPIO 28-30  SPI1 MISO/MOSI/CLK (Encoder)
```

This troubleshooting guide should help you get from "compiled" to "working" quickly!
