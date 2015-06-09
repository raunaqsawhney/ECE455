#include <lpc17xx.h>
#include "glcd.h"
#include <stdio.h>

char curr_state[16];
int input;
int global_time;
char counter[64];

typedef enum {
	S0,		// 0
	S1,		// 1
	S2,		// 2
	S3,		// 3
	S4,		// 4
	S5,		// 5
	S6,		// 6
	S7		// 7
} state;

typedef enum {
	DOT,	// 0
	DASH,	// 1
} event;
	
state transition_matrix[8][2] = {
	{S1, S0},
	{S0, S2},
	{S1, S3},
	{S4, S0},
	{S1, S5},
	{S6, S3},
	{S7, S2},
	{S7, S7}
};

void init_system()
{
	// Initialize system and GLCD
	SystemInit();
	GLCD_Init();
	GLCD_Clear(White);
}

void init_int0()
{
	// Set INT0 as input
	LPC_PINCON->PINSEL4 &= ~(3<<20);		// P2.10 is GPIO
	LPC_GPIO2->FIODIR		&= ~(1<<1);		// P2.10 is input
	
	LPC_GPIOINT->IO2IntEnF |= 1 << 10; // falling edge of P2.10
	LPC_GPIOINT->IO2IntEnR |= 1 << 10; // rising edge of P2.10
	NVIC_EnableIRQ(EINT3_IRQn);
}

void EINT3_IRQHandler()
{
	int int0_val;
	volatile int tim0_val;
	
	// Clear the interrupt on EINT0
	LPC_GPIOINT->IO2IntClr |= 1 << 10;
	
	int0_val =~ (LPC_GPIO2->FIOPIN >> 10) & 0x01;
	
	if (int0_val == 1)
	{
		// Start Timer
	  LPC_TIM0->TCR = 0x02;
		LPC_TIM0->TCR = 0x01;
		LPC_TIM0->PR  = 0x00; 
	}
	else if (int0_val == 0)
	{
		tim0_val = LPC_TIM0->TC;
		// Stop Timer
		LPC_TIM0->TCR = 0x02;

	}
	
	global_time = tim0_val;
	
}

int main (void)
{
	int pattern_match = 0;
	state current_state = S0;
	event current_event;
	
	init_system();
	init_int0();

	while (1)
	{
		GLCD_DisplayString(7,1,1, "");
		sprintf(counter, "%d", global_time);
		GLCD_DisplayString(7,1,1, (unsigned char *)counter);
	}
	
	while (!pattern_match)
	{
		if (input == 0)
			current_event = DOT;
		else if (input == 1)
			current_event = DASH;
		
		current_state = transition_matrix[current_state][current_event];
		
		sprintf(curr_state, "%02d", current_state);
		GLCD_DisplayString(7, 1, 1, (unsigned char *)curr_state);
		
		if (current_state == S7)
		{
			pattern_match = 1;
			GLCD_DisplayString(7,1,1,"CORRECT");
		}
	}
}
