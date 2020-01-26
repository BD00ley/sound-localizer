#include <xc.h>
#include "xc8.h"

#define BR	0
#define BL	1
#define TOP 2

void motorDrive (volatile unsigned char *PORT, unsigned short steps, unsigned char *Polarity);