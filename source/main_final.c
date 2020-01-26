/*	
  ____                _ _               _____              _            
 |  _ \              | | |             |  __ \            | |           
 | |_) |_ __ __ _  __| | | ___ _   _   | |  | | ___   ___ | | ___ _   _ 
 |  _ <| '__/ _` |/ _` | |/ _ \ | | |  | |  | |/ _ \ / _ \| |/ _ \ | | |
 | |_) | | | (_| | (_| | |  __/ |_| |  | |__| | (_) | (_) | |  __/ |_| |
 |____/|_|  \__,_|\__,_|_|\___|\__, |  |_____/ \___/ \___/|_|\___|\__, |
                                __/ |                              __/ |
                               |___/                              |___/ 
							   
	This is final version code for a sound localizer.
	This code is written to accept three analog inputs
	through a microphone, then drive a stepper motor to the 
	direction sound is coming from (either BL, BR or TOP).
	In addition, the the microcontroller accepts commands
	from an ESP8266 chip and interfaces with an I2C LCD.
	The ESP8266 setups up a wifi point which can be accessed
	to initialize the stepper motor or make the localizer
	listen more closely in a direction. The LCD will display
	text which tells where it thinks sound is coming from.
 */

#include <xc.h>
#include "xc8.h"
#include "stepperMotor.h"
#include "USART_functions.h"
#include "lcd_i2c.h"
unsigned char Direction = BR;	//Register to track motor direction. The motor needs to be calibrated before hand to point BR.
volatile unsigned char *lata_star;
unsigned char *motorPolarity_pointer;

unsigned char MicIn01_thres = 0xD0;
unsigned char MicIn02_thres = 0xD0;
unsigned char MicIn03_thres = 0xD0;

void main(void) {
	
	unsigned char MicIn01_low  = 0;
    unsigned char MicIn01_high = 0;
	unsigned char MicIn02_low  = 0;
    unsigned char MicIn02_high = 0;
	unsigned char MicIn03_low  = 0;
	unsigned char MicIn03_high = 0;
	
	unsigned char motorPolarity = 0b0011;
    
    lata_star =  &LATA;						//Point to register for LATA
	motorPolarity_pointer = &motorPolarity;	//Point to register for motorPolarity
    
	OSCCON = 0x62;	//8 MHz internal
	INTCONbits.GIE = 1;		//Allow global interrupts
	INTCONbits.PEIE = 1;	//Enable interrupts for peripherals (UART)
	PIE1bits.RCIE	= 1;	//Enable UART receive interrupt
	
	ANSELA = 0;	//Digital inputs
	ANSELB = 0;
	TRISDbits.TRISD5 = 1;	//Input01 microphone
    PORTDbits.RD5    = 0;
	TRISDbits.TRISD6 = 1;	//Input02 microphone
    PORTDbits.RD6    = 0;
	TRISDbits.TRISD7 = 1;	//Input03 microphone
    PORTDbits.RD7    = 0;
	
    PORTA = 0;
	TRISAbits.TRISA0 = 0;	// Motor Outputs
	TRISAbits.TRISA1 = 0;	
	TRISAbits.TRISA2 = 0;
	TRISAbits.TRISA3 = 0;
	
	ANSELDbits.ANSD7 = 1;	//Analog input
	ANSELDbits.ANSD6 = 1;	//Analog input
	ANSELDbits.ANSD5 = 1;	//Analog input
	
	ADCON0 = 0b01100100;	//Analog channel 25 (port D5), ADC is off initially
	ADCON1 = 0b00000000;	//All internal signals for reference voltage
	ADCON2 = 0b10101000;	//BR justified, 12 TAD, Fosc/16
	
	LATB = 0;
	PORTB = 0;
	I2C_init(100000);
	I2C_LCD_command_setup();
	I2C_LCD_string(" Ready.");
	USART_setup();
	USART_start();
	
	ADCON0bits.ADON = 1;	//Start the ADC conversion
    ADCON0bits.GO = 1;
	while(1)
	{
		if(ADCON0bits.GO == 0)
		{
			MicIn01_low = ADRESL;		//Store the sampled inputs
			MicIn01_high = ADRESH;
			
			ADCON0bits.CHS = 0b11010;	//Change sampling channel to 26 (Port D6)
            __delay_us(1);
			ADCON0bits.GO = 1;
			while(ADCON0bits.GO == 1)		//Wait until sampling is done
				continue;
			MicIn02_low = ADRESL;		//Store the new sampled inputs
			MicIn02_high = ADRESH;
			
			ADCON0bits.CHS = 0b11011;	//Change sampling channel to 27 (Port D7)
            __delay_us(1);
			ADCON0bits.GO = 1;
			while(ADCON0bits.GO == 1)		//Wait until sampling is done
				continue;
			MicIn03_low = ADRESL;		//Store the new sampled inputs
			MicIn03_high = ADRESH;
			
			if((MicIn01_high > MicIn02_high) && (MicIn01_high > MicIn03_high))
			{
				if(MicIn01_high >= 1 && MicIn01_low >= MicIn01_thres && Direction != BR)   //Voltage is at least 2.5V and the direction is BR; 
				{
					I2C_LCD_send(0x01,0, 0x4E);
					I2C_LCD_send(0x80,0, 0x4E);	
					I2C_LCD_string(" I hear something at");
					I2C_LCD_send(0xC0,0, 0x4E);
					I2C_LCD_string("bottom right.");
					//Spin the motor to point bottom right
					if(Direction == TOP)
						motorDrive(lata_star, 172, motorPolarity_pointer);	//Amount of steps required to get from top to bottom right
					else
						motorDrive(lata_star, 342, motorPolarity_pointer);	//Amount of steps required to get from bottom left to bottom right
                    Direction = BR;
				}   
			}
			else if((MicIn02_high > MicIn01_high) && (MicIn02_high > MicIn03_high))
			{
				if(MicIn02_high >= 1 && MicIn02_low >= MicIn02_thres && Direction != BL)    //Voltage is at least 2.5V and the direction is BL
				{
					I2C_LCD_send(0x01,0, 0x4E);
					I2C_LCD_send(0x80,0, 0x4E);	
					I2C_LCD_string(" I hear something at");
					I2C_LCD_send(0xC0,0, 0x4E);
					I2C_LCD_string("bottom left.");
					//Spin the motor to point bottom left
					if(Direction == BR)
						motorDrive(lata_star, 172, motorPolarity_pointer);	//Amount of steps required to get from bottom right to bottom left
					else
						motorDrive(lata_star, 342, motorPolarity_pointer);	//Amount of steps required to get from top to bottom left
                    Direction = BL;
				}   
			}
			else if ((MicIn03_high > MicIn01_high) && (MicIn03_high > MicIn02_high))
			{
				if(MicIn03_high >= 1 && MicIn03_low >= MicIn03_thres && Direction != TOP)    //Voltage is at least 2.5V and the direction is BL
				{
					I2C_LCD_send(0x01,0, 0x4E);
					I2C_LCD_send(0x80,0, 0x4E);	
					I2C_LCD_string(" I hear something at");
					I2C_LCD_send(0xC0,0, 0x4E);
					I2C_LCD_string("the top.");
					//Spin the motor to point top-side
					if(Direction == BL)
						motorDrive(lata_star, 172, motorPolarity_pointer);	//Amount of steps required to get from bottom left to top
					else
						motorDrive(lata_star, 342, motorPolarity_pointer);	//Amount of steps required to get from bottom right to top
                    Direction = TOP;
				}   
			}
			else
			{
                if((MicIn01_low > MicIn02_low) && (MicIn01_low > MicIn03_high))
                {
                    if(MicIn01_high >= 1 && MicIn01_low >= MicIn01_thres && Direction != BR)    
					{
						I2C_LCD_send(0x01,0, 0x4E);
						I2C_LCD_send(0x80,0, 0x4E);	
						I2C_LCD_string(" I hear something at");
						I2C_LCD_send(0xC0,0, 0x4E);
						I2C_LCD_string("at bottom right.");
						//Spin the motor to point bottom right
						if(Direction == TOP)
							motorDrive(lata_star, 172, motorPolarity_pointer);	//Amount of steps required to get from top to bottom right
						else
							motorDrive(lata_star, 342, motorPolarity_pointer);	//Amount of steps required to get from bottom left to bottom right
						Direction = BR;
					}   
                }
				else if((MicIn02_low > MicIn01_low) && (MicIn02_low > MicIn03_low))
				{
					if(MicIn02_high >= 1 && MicIn02_low >= MicIn02_thres && Direction != BL)    //Voltage is at least 2.5V and the direction is BL
					{
						I2C_LCD_send(0x01,0, 0x4E);
						I2C_LCD_send(0x80,0, 0x4E);	
						I2C_LCD_string(" I hear something at");
						I2C_LCD_send(0xC0,0, 0x4E);
						I2C_LCD_string("at bottom left.");
						//Spin the motor to point bottom left
						if(Direction == BR)
							motorDrive(lata_star, 172, motorPolarity_pointer);	//Amount of steps required to get from bottom right to bottom left
						else
							motorDrive(lata_star, 342, motorPolarity_pointer);	//Amount of steps required to get from top to bottom left
						Direction = BL;
					}   
				}
				else if((MicIn03_low > MicIn02_low) && (MicIn03_low > MicIn01_low))
				{
					if(MicIn03_high >= 1 && MicIn03_low >= MicIn03_thres && Direction != TOP)    //Voltage is at least 2.5V and the direction is BL
					{
						I2C_LCD_send(0x01,0, 0x4E);
						I2C_LCD_send(0x80,0, 0x4E);	
						I2C_LCD_string(" I hear something");
						I2C_LCD_send(0xC0,0, 0x4E);
						I2C_LCD_string("at the top.");
						//Spin the motor to point top-side
						if(Direction == BL)
							motorDrive(lata_star, 172, motorPolarity_pointer);	//Amount of steps required to get from bottom left to top
						else
							motorDrive(lata_star, 342, motorPolarity_pointer);	//Amount of steps required to get from bottom right to top
						Direction = TOP;
					}   
				}
			}
			ADCON0bits.CHS = 0b11001;	//Change back to channel 25 (Port D5)
			ADCON0bits.GO = 1;
		}
	}
    return;
}

void __interrupt(low_priority) Read_Data(void)
{
	char DATA = 0;
	if(RCIF == 1)
	{
		if(RCSTAbits.OERR)
		{           
			CREN = 0;
			NOP();
			CREN=1;
		}	
		DATA = RCREG;  //Store RCREG into register DATA
	}	
		
	if(DATA == 0x2A && Direction != BL)
	{
			//Spin the motor to point bottom left
			if(Direction == BR)
				motorDrive(lata_star, 172, motorPolarity_pointer);	//Amount of steps required to get from bottom right to bottom left
			else
				motorDrive(lata_star, 342, motorPolarity_pointer);	//Amount of steps required to get from top to bottom left
			I2C_LCD_send(0x01,0, 0x4E);
			I2C_LCD_send(0x80,0, 0x4E);	
			I2C_LCD_string(" Ready at the");
			I2C_LCD_send(0xC0,0, 0x4E);
			I2C_LCD_string("bottom left.");
			Direction = BL;
	}
	else if(DATA == 0x2B && Direction != BR)
	{
			//Spin the motor to point bottom right
			if(Direction == TOP)
				motorDrive(lata_star, 172, motorPolarity_pointer);	//Amount of steps required to get from top to bottom right
			else
				motorDrive(lata_star, 342, motorPolarity_pointer);	//Amount of steps required to get from bottom left to bottom right
			I2C_LCD_send(0x01,0, 0x4E);
			I2C_LCD_send(0x80,0, 0x4E);	
			I2C_LCD_string(" Ready at the");
			I2C_LCD_send(0xC0,0, 0x4E);
			I2C_LCD_string("bottom right.");
            Direction = BR; 
	}
	else if (DATA == 0x2C && Direction != TOP)
	{
		if(Direction != TOP)
		{
			//Spin the motor to point top-side
			if(Direction == BL)
				motorDrive(lata_star, 172, motorPolarity_pointer);	//Amount of steps required to get from bottom left to top
			else
				motorDrive(lata_star, 342, motorPolarity_pointer);	//Amount of steps required to get from bottom right to top
			Direction = TOP;
			I2C_LCD_send(0x01,0, 0x4E);
			I2C_LCD_send(0x80,0, 0x4E);	
			I2C_LCD_string(" Ready at the");
			I2C_LCD_send(0xC0,0, 0x4E);
			I2C_LCD_string("top.");
		}
	}
	else if(DATA == 0x2D)	//Listen closer to bottom left
	{
			MicIn01_thres = 0xF4;
			MicIn02_thres = 0xD0;
			MicIn03_thres = 0xF4;
			I2C_LCD_send(0x01,0, 0x4E);
			I2C_LCD_send(0x80,0, 0x4E);	
			I2C_LCD_string(" Listening closer");
			I2C_LCD_send(0xC0,0, 0x4E);
			I2C_LCD_string("at bottom left.");
	}
	else if(DATA == 0x3C)	//Listen closer to bottom right
	{
			MicIn01_thres = 0xD0;
			MicIn02_thres = 0xF4;
			MicIn03_thres = 0xF4;
			I2C_LCD_send(0x01,0, 0x4E);
			I2C_LCD_send(0x80,0, 0x4E);	
			I2C_LCD_string(" Listening closer");
			I2C_LCD_send(0xC0,0, 0x4E);
			I2C_LCD_string("at bottom right.");
	}
	else if(DATA == 0x3E)	//Listen closer to the top
	{
			MicIn01_thres = 0xF4;
			MicIn02_thres = 0xF4;
			MicIn03_thres = 0xD0;
			I2C_LCD_send(0x01,0, 0x4E);
			I2C_LCD_send(0x80,0, 0x4E);	
			I2C_LCD_string(" Listening closer");
			I2C_LCD_send(0xC0,0, 0x4E);
			I2C_LCD_string("at top.");
	}
	RCIF = 0;
}
