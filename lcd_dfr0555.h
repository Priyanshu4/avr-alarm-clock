/*
 * lcd_dfr0555.h
 *
 * Created: 4/16/2025 1:59:57 PM
 *  Author: agpri
 */ 

#include <stdint.h>

// Send command to LCD
void LCD_command(uint8_t cmd);

// Send one character to LCD
void LCD_data(uint8_t data);

// Write to backlight
void LCD_backlight_write(uint8_t cmd, uint8_t data);

// Initialize LCD (2-line, 5x8 dots, display on, clear)
void LCD_init();

// Print a string to LCD
void LCD_print(const char* str);

// Print a string to LCD on a given line
void LCD_printline(const char* str, uint8_t line);

// Print a string to LCD centered on a given line
void LCD_printline_centered(const char* str, uint8_t line);

// Clear LCD
void LCD_clear();

// Clear LCD
void LCD_clearline(uint8_t line);

// Set cursor position
void LCD_set_cursor(uint8_t row, uint8_t col);

void LCD_display_on_off(uint8_t display, uint8_t cursor, uint8_t blinking_cursor);