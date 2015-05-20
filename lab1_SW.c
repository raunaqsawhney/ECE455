#include <lpc17xx.h>
#include "glcd.h"
#include <stdio.h>

uint32_t g_milliseconds;

void delay_sw(uint32_t milliseconds)
{
	volatile int i, j;
	char time[128];
	
	for (i=0; i<=milliseconds;i++)
	{
		for (j=0; j<16200;j++)
		{
			__nop();
		}
		
		sprintf(time, "%02d:%02d",(i/60000),(i/1000)%60);
		
		if (i % 1000 == 0)
		{
			 GLCD_DisplayString(7, 1, 1, (unsigned char *)time);
		}
	}
}

int main(void)
{
	SystemInit();
	GLCD_Init();
	GLCD_Clear(White);
	__disable_irq();
	delay_sw(600000);
	__enable_irq();
	return 0;
}
