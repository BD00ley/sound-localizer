#include <xc.h>
#include "xc8.h"
#include <stdio.h>
#include <pic18f45k50.h>

void I2C_LCD_command_setup(void);
void I2C_idle(void);
void I2C_wait(void);
void I2C_init(const unsigned long);
void I2C_start(void);
void I2C_write(unsigned char);
void I2C_stop(void);
void I2C_LCD_send(unsigned char, unsigned char, unsigned char);
void I2C_LCD_string(const char*);
void LCD_4bit_reset(void);