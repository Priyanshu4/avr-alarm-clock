/*
 * lcd_dfr0555.c
 *
 * Created: 4/16/2025 2:02:03 PM
 *  Author: agpri
 */ 
#define F_CPU 16000000UL

#include "lcd_dfr0555.h"
#include "i2c_lib_S25.h"
#include <util/delay.h>
#include <string.h>

#define LCD_ADDRESS		  0x3E
#define BACKLIGHT_ADDRESS 0x6B
#define LCD_DATA_CTRL 0x40
#define LCD_CMD_CTRL 0x00



// Send command to LCD
void LCD_command(uint8_t cmd) {
	TWI_Address(LCD_ADDRESS, TW_WRITE);
	TWI_Transmit_Data(LCD_CMD_CTRL); 
	TWI_Transmit_Data(cmd);
	TWI_Stop();
}

// Send one character to LCD
void LCD_data(uint8_t data) {
	TWI_Address(LCD_ADDRESS, TW_WRITE);
	TWI_Transmit_Data(LCD_DATA_CTRL); // Control byte for data
	TWI_Transmit_Data(data);
	TWI_Stop();
}

// Write to backlight
void LCD_backlight_write(uint8_t cmd, uint8_t data) {
	TWI_Address(BACKLIGHT_ADDRESS, TW_WRITE);
	TWI_Transmit_Data(cmd); 
	TWI_Transmit_Data(data);
	TWI_Stop();
}

// Initialize LCD (2-line, 5x8 dots, display on, clear).
// TWI_Host_Initialize must be called first.
void LCD_init() {
	LCD_display_on_off(1, 0, 0);
	_delay_us(39);
	// LCD 2-line on
	LCD_command(0b00101000);	// Function set: 8-bit, 2-line, 5x8 dots
	_delay_us(39);
	//Reset Register for Backlight
	LCD_backlight_write(0x2F, 0x00);
	//Shutdown Register Write
	LCD_backlight_write(0x00, 0b00100000);
	//PWM Register Write to Full Blue
	LCD_backlight_write(0x04, 0xFF);
	// Send Update PWM
	LCD_backlight_write(0x07, 0x00);
}

// Print a string to LCD
void LCD_print(const char* str) {
	while (*str) {
		LCD_data(*str++);
	}
}

// Print a string to LCD on a given line
void LCD_printline(const char* str, uint8_t line) {
	size_t len = strlen(str);
	LCD_set_cursor(0, line);
	while (*str) {
		LCD_data(*str++);
	}
	for (size_t i = len; i < 16; i++) {
		LCD_data(' ');
	}
}

// Print a string to LCD on a given line
void LCD_printline_centered(const char* str, uint8_t line) {
	size_t len = strlen(str);
	size_t leftpad = 0;
	size_t rightpad = 0;
	if (len < 16) {
		leftpad = (16 - len) / 2;
		rightpad = 16 - len - leftpad;
	}
	LCD_set_cursor(0, line);
	for (size_t i = 0; i < leftpad; i++) {
		LCD_data(' ');
	}
	while (*str) {
		LCD_data(*str++);
	}
	for (size_t i = 0; i < rightpad; i++) {
		LCD_data(' ');
	}
}

void LCD_clear(void){
	LCD_command(0x01);
	_delay_ms(1.53);
}


// Set cursor position
void LCD_set_cursor(uint8_t row, uint8_t col) {
	LCD_command(0b10000000 | (row + 0x40*col));
}

void LCD_display_on_off(uint8_t display, uint8_t cursor, uint8_t blinking_cursor) {
	uint8_t cmd = 0b1000;
	
	if (display) {
		cmd |= 0b100;
	}
	
	if (cursor) {
		cmd |= 0b010;
	}
	
	if (blinking_cursor) {
		cmd |= 0b001;
	}
	
	LCD_command(cmd);
}

