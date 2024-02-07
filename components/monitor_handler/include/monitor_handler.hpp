#pragma once

#include <string.h>

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/task.h"
#include "freertos/FreeRTOS.h"


#include "sdkconfig.h" // generated by "make menuconfig"

#ifndef MAIN_SSD1366_H_
#define MAIN_SSD1366_H_
#endif
// Following definitions are bollowed from 
// http://robotcantalk.blogspot.com/2015/03/interfacing-arduino-with-ssd1306-driven.html

// SLA (0x3C) + WRITE_MODE (0x00) =  0x78 (0b01111000)
#define OLED_I2C_ADDRESS   0x3C

// Control byte
#define OLED_CONTROL_BYTE_CMD_SINGLE    0x80
#define OLED_CONTROL_BYTE_CMD_STREAM    0x00
#define OLED_CONTROL_BYTE_DATA_STREAM   0x40

// Fundamental commands (pg.28)
#define OLED_CMD_SET_CONTRAST           0x81    // follow with 0x7F
#define OLED_CMD_DISPLAY_RAM            0xA4
#define OLED_CMD_DISPLAY_ALLON          0xA5
#define OLED_CMD_DISPLAY_NORMAL         0xA6
#define OLED_CMD_DISPLAY_INVERTED       0xA7
#define OLED_CMD_DISPLAY_OFF            0xAE
#define OLED_CMD_DISPLAY_ON             0xAF

// Addressing Command Table (pg.30)
#define OLED_CMD_SET_MEMORY_ADDR_MODE   0x20    // follow with 0x00 = HORZ mode = Behave like a KS108 graphic LCD
#define OLED_CMD_SET_COLUMN_RANGE       0x21    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x7F = COL127
#define OLED_CMD_SET_PAGE_RANGE         0x22    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x07 = PAGE7

// Hardware Config (pg.31)
#define OLED_CMD_SET_DISPLAY_START_LINE 0x40
#define OLED_CMD_SET_SEGMENT_REMAP      0xA1    
#define OLED_CMD_SET_MUX_RATIO          0xA8    // follow with 0x3F = 64 MUX
#define OLED_CMD_SET_COM_SCAN_MODE      0xC8    
#define OLED_CMD_SET_DISPLAY_OFFSET     0xD3    // follow with 0x00
#define OLED_CMD_SET_COM_PIN_MAP        0xDA    // follow with 0x12
#define OLED_CMD_NOP                    0xE3    // NOP

// Timing and Driving Scheme (pg.32)
#define OLED_CMD_SET_DISPLAY_CLK_DIV    0xD5    // follow with 0x80
#define OLED_CMD_SET_PRECHARGE          0xD9    // follow with 0xF1
#define OLED_CMD_SET_VCOMH_DESELCT      0xDB    // follow with 0x30

// Charge Pump (pg.62)
#define OLED_CMD_SET_CHARGE_PUMP        0x8D    // follow with 0x14


#ifndef MAIN_FONT8X8_BASIC_H_
#define MAIN_FONT8X8_BASIC_H_
#endif /* MAIN_FONT8X8_BASIC_H_ */

#define SDA_PIN GPIO_NUM_21
#define SCL_PIN GPIO_NUM_22

class MonitorHandler{
    static const char *TAG;
    private:
        static int mode; // 0: data display, 1: communicate display
        static int temperature;
        static int humidity;
        static int last_page;
        static char *communicate;
        static void i2cMasterInit();
        static void ssd1306Init();
        static void clearDisplay();
        static void printText(const char *text);
        
    public:
        static void destroyCapitalism();
        static void initializeMonitor();
        static void updateData(const int temperature, const int humidity, const float moisture);
        static void updateCommunicate(const char *communicate, const int after_data);

};



