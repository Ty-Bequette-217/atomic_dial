#ifndef ARDUINO_COMPAT_H
#define ARDUINO_COMPAT_H

#include <ctype.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"

#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define ANALOG 3
#define CHANGE 1
#define RISING 2
#define FALLING 3
#define MSBFIRST 1
#define LSBFIRST 0
#define SPI_MODE0 0
#define SPI_MODE1 1
#define SPI_MODE2 2
#define SPI_MODE3 3

typedef uint8_t byte;
typedef uint16_t word;
typedef bool boolean;

class __FlashStringHelper;
typedef const char *StringSumHelper;

#define F(str) reinterpret_cast<const __FlashStringHelper *>(str)

template <typename T>
constexpr const T &min(const T &a, const T &b) {
    return (b < a) ? b : a;
}

template <typename T>
constexpr const T &max(const T &a, const T &b) {
    return (a < b) ? b : a;
}

inline void pinMode(int pin, int mode) {
    gpio_init((uint)pin);
    if (mode == OUTPUT) {
        gpio_set_dir((uint)pin, GPIO_OUT);
    } else {
        gpio_set_dir((uint)pin, GPIO_IN);
        if (mode == INPUT_PULLUP) {
            gpio_pull_up((uint)pin);
        }
    }
}

inline void digitalWrite(int pin, int value) {
    gpio_put((uint)pin, value ? 1 : 0);
}

inline int digitalRead(int pin) {
    return gpio_get((uint)pin) ? HIGH : LOW;
}

inline int analogRead(int pin) {
    (void)pin;
    return 0;
}

inline void analogWrite(int pin, int value) {
    (void)pin;
    (void)value;
}

inline unsigned long micros(void) {
    return (unsigned long)to_us_since_boot(get_absolute_time());
}

inline unsigned long millis(void) {
    return micros() / 1000UL;
}

inline void delay(unsigned long ms) {
    sleep_ms(ms);
}

inline void delayMicroseconds(unsigned int us) {
    sleep_us(us);
}

inline int digitalPinToInterrupt(int pin) {
    return pin;
}

inline void attachInterrupt(int interrupt, void (*callback)(void), int mode) {
    (void)interrupt;
    (void)callback;
    (void)mode;
}

inline void detachInterrupt(int interrupt) {
    (void)interrupt;
}

inline void noInterrupts(void) {
}

inline void interrupts(void) {
}

inline unsigned long pulseIn(int pin, int state, unsigned long timeout = 1000000UL) {
    (void)pin;
    (void)state;
    (void)timeout;
    return 0;
}

inline bool isDigit(char c) {
    return isdigit((unsigned char)c) != 0;
}

class Print {
public:
    virtual ~Print() = default;

    virtual size_t write(uint8_t value) {
        putchar(value);
        return 1;
    }

    size_t print(const char *value) {
        if (value == nullptr) {
            return 0;
        }
        return (size_t)printf("%s", value);
    }

    size_t print(const __FlashStringHelper *value) {
        return print(reinterpret_cast<const char *>(value));
    }

    size_t print(char value) {
        return write((uint8_t)value);
    }

    size_t print(int value) {
        return (size_t)printf("%d", value);
    }

    size_t print(long value) {
        return (size_t)printf("%ld", value);
    }

    size_t print(float value) {
        return (size_t)printf("%f", (double)value);
    }

    size_t print(float value, int decimals) {
        if (decimals < 0) {
            decimals = 0;
        }
        if (decimals > 9) {
            decimals = 9;
        }
        return (size_t)printf("%.*f", decimals, (double)value);
    }

    size_t print(double value, int decimals) {
        return print((float)value, decimals);
    }

    size_t println(void) {
        return print("\n");
    }

    size_t println(float value, int decimals) {
        size_t written = print(value, decimals);
        return written + println();
    }

    size_t println(double value, int decimals) {
        return println((float)value, decimals);
    }

    template <typename T>
    size_t println(T value) {
        size_t written = print(value);
        return written + println();
    }
};

class Stream : public Print {
public:
    virtual int available(void) {
        return 0;
    }

    virtual int read(void) {
        return -1;
    }
};

class HardwareSerial : public Stream {
public:
    void begin(unsigned long baud) {
        (void)baud;
    }
};

static HardwareSerial Serial;

#endif // ARDUINO_COMPAT_H
