#include "display.h"
#include "board_config.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"

// --- LOW LEVEL HARDWARE FUNCTIONS ---

static void lcd_write_cmd(uint8_t cmd) {
    gpio_put(LCD_CMD_PIN, 0); 
    gpio_put(LCD_CS_PIN, 0); 
    spi_write_blocking(LCD_SPI_CHAN, &cmd, 1);
    gpio_put(LCD_CS_PIN, 1); 
}

static void lcd_write_data(uint8_t data) {
    gpio_put(LCD_CMD_PIN, 1); 
    gpio_put(LCD_CS_PIN, 0); 
    spi_write_blocking(LCD_SPI_CHAN, &data, 1);
    gpio_put(LCD_CS_PIN, 1); 
}

static void lcd_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    lcd_write_cmd(0x2A); 
    lcd_write_data(x0 >> 8); lcd_write_data(x0 & 0xFF);
    lcd_write_data(x1 >> 8); lcd_write_data(x1 & 0xFF);
    lcd_write_cmd(0x2B); 
    lcd_write_data(y0 >> 8); lcd_write_data(y0 & 0xFF);
    lcd_write_data(y1 >> 8); lcd_write_data(y1 & 0xFF);
    lcd_write_cmd(0x2C); 
}

// --- INITIALIZATION ---

void lcd_init(void) {
    spi_init(LCD_SPI_CHAN, 20000000);
    
    gpio_set_function(LCD_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(LCD_DATA_PIN, GPIO_FUNC_SPI);

    gpio_init(LCD_CS_PIN); gpio_set_dir(LCD_CS_PIN, GPIO_OUT); gpio_put(LCD_CS_PIN, 1); 
    gpio_init(LCD_CMD_PIN); gpio_set_dir(LCD_CMD_PIN, GPIO_OUT);
    gpio_init(LCD_RST_PIN); gpio_set_dir(LCD_RST_PIN, GPIO_OUT);
    gpio_init(LCD_BACKLIGHT_PIN); gpio_set_dir(LCD_BACKLIGHT_PIN, GPIO_OUT);
    
    gpio_put(LCD_BACKLIGHT_PIN, 1); 

    gpio_put(LCD_RST_PIN, 1); sleep_ms(10);
    gpio_put(LCD_RST_PIN, 0); sleep_ms(10);
    gpio_put(LCD_RST_PIN, 1); sleep_ms(120); 

    lcd_write_cmd(0xEF); 
    lcd_write_cmd(0xEB); lcd_write_data(0x14); 
    
    // --- GC9A01 Specific Initialization Registers ---
    lcd_write_cmd(0xEB); lcd_write_data(0x14); 
    lcd_write_cmd(0xFE); lcd_write_cmd(0xEF); 
    lcd_write_cmd(0xEB); lcd_write_data(0x14); 
    lcd_write_cmd(0x84); lcd_write_data(0x40); 
    lcd_write_cmd(0x85); lcd_write_data(0xFF); 
    lcd_write_cmd(0x86); lcd_write_data(0xFF); 
    lcd_write_cmd(0x87); lcd_write_data(0xFF); 
    lcd_write_cmd(0x88); lcd_write_data(0x0A); 
    lcd_write_cmd(0x89); lcd_write_data(0x21); 
    lcd_write_cmd(0x8A); lcd_write_data(0x00); 
    lcd_write_cmd(0x8B); lcd_write_data(0x80); 
    lcd_write_cmd(0x8C); lcd_write_data(0x01); 
    lcd_write_cmd(0x8D); lcd_write_data(0x01); 
    lcd_write_cmd(0x8E); lcd_write_data(0xFF); 
    lcd_write_cmd(0x8F); lcd_write_data(0xFF); 

    lcd_write_cmd(0x3A); lcd_write_data(0x05); 
    lcd_write_cmd(0x36); lcd_write_data(0x48); 

    lcd_write_cmd(0x90); lcd_write_data(0x08); lcd_write_data(0x08); lcd_write_data(0x08); lcd_write_data(0x08); 
    lcd_write_cmd(0xBD); lcd_write_data(0x06);
    lcd_write_cmd(0xBC); lcd_write_data(0x00);
    lcd_write_cmd(0xFF); lcd_write_data(0x60); lcd_write_data(0x01); lcd_write_data(0x04);
    lcd_write_cmd(0xC3); lcd_write_data(0x13); lcd_write_cmd(0xC4); lcd_write_data(0x13);
    lcd_write_cmd(0xC9); lcd_write_data(0x22);
    lcd_write_cmd(0xBE); lcd_write_data(0x11);
    lcd_write_cmd(0xE1); lcd_write_data(0x10); lcd_write_data(0x0E);
    lcd_write_cmd(0xDF); lcd_write_data(0x21); lcd_write_data(0x0c); lcd_write_data(0x02);
    lcd_write_cmd(0xF0); lcd_write_data(0x45); lcd_write_data(0x09); lcd_write_data(0x08); lcd_write_data(0x08); lcd_write_data(0x26); lcd_write_data(0x2A);
    lcd_write_cmd(0xF1); lcd_write_data(0x43); lcd_write_data(0x70); lcd_write_data(0x72); lcd_write_data(0x36); lcd_write_data(0x37); lcd_write_data(0x6F);
    lcd_write_cmd(0xF2); lcd_write_data(0x45); lcd_write_data(0x09); lcd_write_data(0x08); lcd_write_data(0x08); lcd_write_data(0x26); lcd_write_data(0x2A);
    lcd_write_cmd(0xF3); lcd_write_data(0x43); lcd_write_data(0x70); lcd_write_data(0x72); lcd_write_data(0x36); lcd_write_data(0x37); lcd_write_data(0x6F);
    lcd_write_cmd(0xED); lcd_write_data(0x1B); lcd_write_data(0x0B);
    lcd_write_cmd(0xAE); lcd_write_data(0x77);
    lcd_write_cmd(0xCD); lcd_write_data(0x63);
    lcd_write_cmd(0x70); lcd_write_data(0x07); lcd_write_data(0x07); lcd_write_data(0x04); lcd_write_data(0x0E); lcd_write_data(0x0F); lcd_write_data(0x09); lcd_write_data(0x07); lcd_write_data(0x08); lcd_write_data(0x03);
    lcd_write_cmd(0xE8); lcd_write_data(0x34);
    lcd_write_cmd(0x62); lcd_write_data(0x18); lcd_write_data(0x0D); lcd_write_data(0x71); lcd_write_data(0xED); lcd_write_data(0x70); lcd_write_data(0x70); lcd_write_data(0x18); lcd_write_data(0x0F); lcd_write_data(0x71); lcd_write_data(0xEF); lcd_write_data(0x70); lcd_write_data(0x70);
    lcd_write_cmd(0x63); lcd_write_data(0x18); lcd_write_data(0x11); lcd_write_data(0x71); lcd_write_data(0xF1); lcd_write_data(0x70); lcd_write_data(0x70); lcd_write_data(0x18); lcd_write_data(0x13); lcd_write_data(0x71); lcd_write_data(0xF3); lcd_write_data(0x70); lcd_write_data(0x70);
    lcd_write_cmd(0x64); lcd_write_data(0x28); lcd_write_data(0x29); lcd_write_data(0xF1); lcd_write_data(0x01); lcd_write_data(0xF1); lcd_write_data(0x00); lcd_write_data(0x07);
    lcd_write_cmd(0x66); lcd_write_data(0x3C); lcd_write_data(0x00); lcd_write_data(0xCD); lcd_write_data(0x67); lcd_write_data(0x45); lcd_write_data(0x45); lcd_write_data(0x10); lcd_write_data(0x00); lcd_write_data(0x00); lcd_write_data(0x00);
    lcd_write_cmd(0x67); lcd_write_data(0x00); lcd_write_data(0x3C); lcd_write_data(0x00); lcd_write_data(0x00); lcd_write_data(0x00); lcd_write_data(0x01); lcd_write_data(0x54); lcd_write_data(0x10); lcd_write_data(0x32); lcd_write_data(0x98);
    lcd_write_cmd(0x74); lcd_write_data(0x10); lcd_write_data(0x85); lcd_write_data(0x80); lcd_write_data(0x00); lcd_write_data(0x00); lcd_write_data(0x4E); lcd_write_data(0x00);
    lcd_write_cmd(0x98); lcd_write_data(0x3e); lcd_write_data(0x07);
    lcd_write_cmd(0x35); lcd_write_cmd(0x21);
    
    lcd_write_cmd(0x11); 
    sleep_ms(120);
    lcd_write_cmd(0x29); 
}

void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
    uint16_t x = area->x1;
    uint16_t y = area->y1;
    uint16_t w = (area->x2 - area->x1 + 1);
    uint16_t h = (area->y2 - area->y1 + 1);

    lcd_set_window(x, y, x + w - 1, y + h - 1);
    
    gpio_put(LCD_CMD_PIN, 1); 
    gpio_put(LCD_CS_PIN, 0);  

    uint32_t total_pixels = w * h;
    for (uint32_t i = 0; i < total_pixels; i++) {
        uint16_t color = color_p->full;
        uint8_t color_high = color >> 8;
        uint8_t color_low = color & 0xFF;
        
        spi_write_blocking(LCD_SPI_CHAN, &color_high, 1);
        spi_write_blocking(LCD_SPI_CHAN, &color_low, 1);
        color_p++; 
    }
    
    gpio_put(LCD_CS_PIN, 1); 
    lv_disp_flush_ready(disp_drv);
}