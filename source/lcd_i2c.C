#include <xc.h>
#include "xc8.h"
#include <stdio.h>
#include <pic18f45k50.h>
#include "lcd_i2c.h"

void I2C_idle()
{
	while((SSP1STAT & 0x04) || (SSP1CON2 & 0x1F));
}
void I2C_wait()
{
	while(!SSPIF);
	SSPIF = 0;
}

void I2C_init(const unsigned long C)
{
	SSP1CON1 = 0b00101000;
	SSP1CON2 = 0;
	SSPADD = (_XTAL_FREQ/(4*C))-1;
	SSPSTAT = 0;
	TRISBbits.TRISB0 = 1;
	TRISBbits.TRISB1 = 1;
	SSPIF = 0;
}
void I2C_start()
{
	I2C_idle();
	SEN = 1;
	I2C_wait();
}
void I2C_write(unsigned char data)
{
	L1: SSP1BUF = data;
	I2C_wait();
	while(ACKSTAT)
	{
		RSEN = 1;
		I2C_wait();
		goto L1;
	}
}

void I2C_stop()
{
	I2C_idle();
	PEN = 1;
	I2C_wait();
	SSPEN= 1;
}

void I2C_LCD_send(unsigned char symbol, unsigned char cmd_or_data, unsigned char Addr)
{
	unsigned char symbol_lsb, symbol_msb;
	symbol_lsb	= ((symbol<<4)&0xF0);
	symbol_msb	=  (symbol&0xF0);
	if(cmd_or_data >= 1)
	{
		I2C_start();
		I2C_write(Addr);
		I2C_write(symbol_msb|0b1101);
		I2C_write(symbol_msb|0b1001);
		I2C_write(symbol_lsb|0b1101);
		I2C_write(symbol_lsb|0b1001);
		I2C_stop();
	}
	else
	{
		I2C_start();
		I2C_write(Addr);
		I2C_write(symbol_msb|0b1100);
		I2C_write(symbol_msb|0b1000);
		I2C_write(symbol_lsb|0b1100);
		I2C_write(symbol_lsb|0b1000);
		I2C_stop();
	}
}
void I2C_LCD_command_setup()
{
	LCD_4bit_reset();
	//I2C_LCD_send(0x04,0,0x4E);		//0x04 Force 2nd line
	//__delay_ms(1);
	I2C_LCD_send(0x38,0, 0x4E);                                           
	I2C_LCD_send(0x01,0, 0x4E);										
    //__delay_ms(1);
	//I2C_LCD_send(0x02,0,0x4E);		//0x0C - 0x0F blinks
	I2C_LCD_send(0x0F,0,0x4E);
	//__delay_ms(1);
	
}
void I2C_LCD_string(const char *string)
{
	while((*string) != '\0')
	{
		I2C_LCD_send(*string, 1, 0x4E);
		string++;
		__delay_ms(1);
	}
}

void LCD_4bit_reset()
{
	__delay_ms(20);						//Begin resest sequence		
	I2C_LCD_send(0x0,0, 0x4E);         
	__delay_ms(10);                     
	I2C_LCD_send(0x0,0, 0x4E);         
	__delay_ms(1);                      
	I2C_LCD_send(0x0,0, 0x4E);         
	__delay_ms(1);                      
	I2C_LCD_send(0x20,0, 0x4E);			//End sequence, 4 bit mode
	__delay_ms(1);
}
