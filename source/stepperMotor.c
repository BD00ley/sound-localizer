#include <xc.h>
#include "xc8.h"
#include "stepperMotor.h"

void motorDrive (volatile unsigned char *PORT, unsigned short steps, unsigned char *Polarity)
{
	unsigned char A = *Polarity;		//Get current polarity of motor in memory
	unsigned char B = (*PORT>>4);		//Upper bits of stepper motor
		for(unsigned short i = 0; i!= steps-1; i++)
		{
			if(A == 0b0011)
                A = 0b0110;
            else if(A == 0b0110)
                A = 0b1100;
            else if(A == 0b1100)
                A = 0b1001;
			else
                A = 0b0011;
            
			__delay_us(6000);
			*PORT = B|A;
		}
    *Polarity = A;	//Update the polarity
}
