/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Standard include. */
#include <stdio.h>

#define S_TO_MS 1000
#define MS_TO_S_SIMULATOR 1680000

#define TASK1_EXECUTION_TIME 1
#define TASK2_EXECUTION_TIME 2
#define TASK3_EXECUTION_TIME 3

#define TASK1_PERIOD 4
#define TASK2_PERIOD 6
#define TASK3_PERIOD 8

/* The ITM port is used to direct the printf() output to the serial window in 
the Keil simulator IDE. */
#define mainITM_Port8(n)    (*((volatile unsigned char *)(0xE0000000+4*n)))
#define mainITM_Port32(n)   (*((volatile unsigned long *)(0xE0000000+4*n)))
#define mainDEMCR           (*((volatile unsigned long *)(0xE000EDFC)))
#define mainTRCENA          0x01000000

// Deadlines of the tasks as required for the assignment
int deadlines[3] = {4,6,8};
/*-----------------------------------------------------------*/

/*
 * The tasks as described in the accompanying PDF application note.
 */
static void edfTask1( void *pvParameters );
static void edfTask2( void *pvParameters );
static void edfTask3( void *pvParameters );

/*
 * Handles for Tasks created
 */
 xTaskHandle task1;
 xTaskHandle task2;
 xTaskHandle task3;

/*
 * Redirects the printf() output to the serial window in the Keil simulator
 * IDE.
 */
int fputc( int iChar, FILE *pxNotUsed );

/*-----------------------------------------------------------*/

/* One array position is used for each task created by this demo.  The 
variables in this array are set and cleared by the trace macros within
FreeRTOS, and displayed on the logic analyzer window within the Keil IDE -
the result of which being that the logic analyzer shows which task is
running when. */
unsigned long ulTaskNumber[ configEXPECTED_NO_RUNNING_TASKS ];

/*-----------------------------------------------------------*/

// EDF Scheduler
void edfScheduler()
{

	// Default priorities, based on earliest deadlines
	vTaskPrioritySet(task1, 3);
	vTaskPrioritySet(task2, 2);
	vTaskPrioritySet(task3, 1);

	// Task 1 > Task 2 > Task 3
	if (deadlines[0] > deadlines[1])
	{
		if (deadlines[1] > deadlines[2])
		{
			vTaskPrioritySet(task3, 3);
			vTaskPrioritySet(task2, 2);
			vTaskPrioritySet(task1, 1);
			return;
		}
	} 
	// Task 2 > Task 1 > Task 3
	else if (deadlines[0] < deadlines[1])
	{
		if (deadlines[0] > deadlines[2])
		{
			vTaskPrioritySet(task3, 3);
			vTaskPrioritySet(task1, 2);
			vTaskPrioritySet(task2, 1);
			return;
		}
	}
	// Task 1 == Task 2
	else if (deadlines[0] == deadlines[1])
	{
		// Task 1 == Task 2 ; Task 3 > Task 1 (and Task 2)
		if (deadlines[0] < deadlines[2])
		{
			vTaskPrioritySet(task1, 3);
			vTaskPrioritySet(task2, 2);
			vTaskPrioritySet(task3, 1);
			return;
		}
		// Task 1 == Task 2 ; Task 1 (and Task 2) > Task 3
		else if (deadlines[0] > deadlines[2])
		{
			vTaskPrioritySet(task3, 3);
			vTaskPrioritySet(task1, 2);
			vTaskPrioritySet(task2, 1);
			return;
		}
	}

	// Task 2 > Task 3 > Task 1
	if (deadlines[1] > deadlines[2])
	{
		if (deadlines[2] > deadlines[0]) 
		{
			vTaskPrioritySet(task1, 3);
			vTaskPrioritySet(task3, 2);
			vTaskPrioritySet(task2, 1);
			return;
		}
	}
	// Task 3 > Task 1 > Task 2
	else if (deadlines[1] < deadlines[2])
	{
		if (deadlines[1] < deadlines[0])
		{
			vTaskPrioritySet(task1, 3);
			vTaskPrioritySet(task2, 2);
			vTaskPrioritySet(task3, 1);
			return;
		}
	}
	// Task 2 == Task 3
	else if (deadlines[1] == deadlines[2])
	{
		// Task 2 == Task 3 ; Task 1 > Task 2 (and Task 3)
		if (deadlines[1] < deadlines[0])
		{
			vTaskPrioritySet(task2, 3);
			vTaskPrioritySet(task3, 2);
			vTaskPrioritySet(task1, 1);
			return;
		}
		// Task 2 == Task 3 ; Task 2 (and Task 3) > Task 1
		else if (deadlines[1] > deadlines[0])
		{
			vTaskPrioritySet(task1, 3);
			vTaskPrioritySet(task2, 2);
			vTaskPrioritySet(task3, 1);
			return;
		}
		// Task 1 == Task 2 == Task 3 ; Default  (assumption)
		if (deadlines[1] == deadlines[2])
		{
			vTaskPrioritySet(task1, 3);
			vTaskPrioritySet(task2, 2);
			vTaskPrioritySet(task3, 1);
			return;
		} 
	}

	// Task 1 > Task 3 > Task 2
	if (deadlines[0] > deadlines[2]) 
	{
		if (deadlines[2] > deadlines[1])
		{
			vTaskPrioritySet(task2, 3);
			vTaskPrioritySet(task3, 2);
			vTaskPrioritySet(task1, 1);
			return;
		}
	}
	// Task 3 > Task 1 > Task 2
	else if (deadlines[0] < deadlines[2])
	{
		if (deadlines[0] > deadlines[1])
		{
			vTaskPrioritySet(task2, 3);
			vTaskPrioritySet(task1, 2);
			vTaskPrioritySet(task3, 1);
			return;
		}
	}
	// Task 1 == Task 3
	else if (deadlines[0] == deadlines[2])
	{
		// Task 1 == Task 3 ; Task 1 (and Task 3) > Task 2
		if (deadlines[0] > deadlines[1])
		{
			vTaskPrioritySet(task2, 3);
			vTaskPrioritySet(task1, 2);
			vTaskPrioritySet(task3, 1); 
			return;
		}
		// Task 1 == Task 3 ; Task 2 > Task 1 (and Task 3)
		else if (deadlines[0] < deadlines[1])
		{
			vTaskPrioritySet(task1, 3);
			vTaskPrioritySet(task3, 2);
			vTaskPrioritySet(task2, 1);
			return;
		}
	}
}

/*-----------------------------------------------------------*/

static void edfTask1( void *pvParameters )
{
	portTickType xNextWakeTime = 0;
	const portTickType xPeriod = TASK1_PERIOD * S_TO_MS;
	long count;

	for( ;; )
	{
		edfScheduler();
		for (count = 0; count < (TASK1_EXECUTION_TIME * MS_TO_S_SIMULATOR); count++){}
		vTaskDelayUntil(&xNextWakeTime, xPeriod);
		deadlines[0] += TASK1_PERIOD;
		edfScheduler();
	}
}

/*-----------------------------------------------------------*/

static void edfTask2( void *pvParameters )
{
	portTickType xNextWakeTime = 0;
	const portTickType xPeriod = TASK2_PERIOD * S_TO_MS;
	long count;

	for( ;; )
	{
		edfScheduler();
		for (count = 0; count < (TASK2_EXECUTION_TIME * MS_TO_S_SIMULATOR); count++){}
		vTaskDelayUntil(&xNextWakeTime, xPeriod);
		deadlines[1] += TASK2_PERIOD;
		edfScheduler();
	}
}

/*-----------------------------------------------------------*/
static void edfTask3( void *pvParameters )
{
	portTickType xNextWakeTime = 0;
	const portTickType xPeriod = TASK3_PERIOD * S_TO_MS;
	long count;

	for( ;; )
	{
		edfScheduler();
		for (count = 0; count < (TASK3_EXECUTION_TIME * MS_TO_S_SIMULATOR); count++){}
		vTaskDelayUntil(&xNextWakeTime, xPeriod);
		deadlines[2] += TASK3_PERIOD;
		edfScheduler();
	}
}

/*-----------------------------------------------------------*/

int fputc( int iChar, FILE *pxNotUsed ) 
{
	/* Just to avoid compiler warnings. */
	( void ) pxNotUsed;

	if( mainDEMCR & mainTRCENA ) 
	{
		while( mainITM_Port32( 0 ) == 0 );
		mainITM_Port8( 0 ) = iChar;
	}

	return( iChar );
}

/*-----------------------------------------------------------*/

int main(void) {

	// Create the tasks as required by the assignment
	xTaskCreate( edfTask1, ( signed char * ) "task1", configMINIMAL_STACK_SIZE, NULL, 3, &task1 );
	xTaskCreate( edfTask2, ( signed char * ) "task2", configMINIMAL_STACK_SIZE, NULL, 2, &task2 );
	xTaskCreate( edfTask3, ( signed char * ) "task3", configMINIMAL_STACK_SIZE, NULL, 1, &task3 );

	// Start the FreeRTOS Default Scheduler
	vTaskStartScheduler();

	// Run indefinately
	while(1){}
	/*-----------------------------------------------------------*/
}
