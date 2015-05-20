#include <lpc17xx.h>
#include "glcd.h"
#include <stdio.h>

uint32_t g_milliseconds;

void delay_hw(uint32_t milliseconds)
{
	char time[64];
	g_milliseconds = 0;
	
	LPC_TIM0->TCR = 0x02;                /* reset timer */
	LPC_TIM0->TCR = 0x01;                /* start timer */
	LPC_TIM0->PR  = 0x00;                /* set prescaler to zero */
	LPC_TIM0->MR0 = 25000;
	LPC_TIM0->MCR |= 0x03;                /* stop timer on match */
	
	NVIC_EnableIRQ(TIMER0_IRQn);
	
	/* wait until delay time has elapsed */
	while (g_milliseconds <= milliseconds+1000)
	{
		sprintf(time, "%02d:%02d", ((g_milliseconds)/60000),((g_milliseconds)/1000)%60);
		GLCD_DisplayString(7, 1, 1, (unsigned char *)time);
	}
}

void TIMER0_IRQHandler()
{
	LPC_TIM0->IR |= 0x01;
	++g_milliseconds;
}
/*
 * Run Simple Timer for 10 Minutes
 */
int main(void)
{
	SystemInit();
	GLCD_Init();
	GLCD_Clear(White);
	delay_hw(600000);
	return 0;
}
