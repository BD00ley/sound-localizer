#include <xc.h>
#include "xc8.h"
#include "USART_functions.h"

void USART_setup(void)
{
	ANSELC = 1;
	TRISCbits.TRISC6 = 1;// TX PIN
	TRISCbits.TRISC7 = 1;// RX PIN
	T2CON=0X01; //Configure timer2 with a prescaler of 4	
	TMR2=0;
	RCSTA1=0x90; //reciever config
	SPBRG = 12;// baud rate close to 9600
}

void USART_start(void)
{
	T2CONbits.TMR2ON=1; //Turn on timer2
}