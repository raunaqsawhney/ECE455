/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Standard include. */
#include <stdio.h>

#define S_TO_MS 1000
#define MS_TO_S_MAGIC_NUMBER 1680000

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
 xTaskHandle T1;
 xTaskHandle T2;
 xTaskHandle T3;

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
    vTaskPrioritySet(T1, 3);
    vTaskPrioritySet(T2, 2);
    vTaskPrioritySet(T3, 1);

    // Task 2 > Task 3 > Task 1
    if ((deadlines[1] > deadlines[2]) && (deadlines[2] > deadlines[0]))
    {
        vTaskPrioritySet(T1, 3);
        vTaskPrioritySet(T3, 2);
        vTaskPrioritySet(T2, 1);
        return;
    }
    // Task 3 > Task 1 > Task 2
    else if ((deadlines[1] < deadlines[2]) && (deadlines[1] < deadlines[0]))
    {
        vTaskPrioritySet(T1, 3);
        vTaskPrioritySet(T2, 2);
        vTaskPrioritySet(T3, 1);
        return;
    }
    // Task 2 == Task 3
    else if (deadlines[1] == deadlines[2])
    {
        // Task 2 == Task 3 ; Task 1 > Task 2 (and Task 3)
        if (deadlines[1] < deadlines[0])
        {
            vTaskPrioritySet(T2, 3);
            vTaskPrioritySet(T3, 2);
            vTaskPrioritySet(T1, 1);
            return;
        }
        // Task 2 == Task 3 ; Task 2 (and Task 3) > Task 1
        else if (deadlines[1] > deadlines[0])
        {
            vTaskPrioritySet(T1, 3);
            vTaskPrioritySet(T2, 2);
            vTaskPrioritySet(T3, 1);
            return;
        }
        // Task 1 == Task 2 == Task 3 ; Default  (assumption)
        if (deadlines[1] == deadlines[2])
        {
            vTaskPrioritySet(T1, 3);
            vTaskPrioritySet(T2, 2);
            vTaskPrioritySet(T3, 1);
            return;
        } 
    }

    // Task 1 > Task 2 > Task 3
    if ((deadlines[0] > deadlines[1]) && (deadlines[1] > deadlines[2]))
    {
        vTaskPrioritySet(T3, 3);
        vTaskPrioritySet(T2, 2);
        vTaskPrioritySet(T1, 1);
        return;
    } 
    // Task 2 > Task 1 > Task 3
    else if ((deadlines[0] < deadlines[1]) && (deadlines[0] > deadlines[2]))
    {
        vTaskPrioritySet(T3, 3);
        vTaskPrioritySet(T1, 2);
        vTaskPrioritySet(T2, 1);
        return;
    }
    // Task 1 == Task 2
    else if (deadlines[0] == deadlines[1])
    {
        // Task 1 == Task 2 ; Task 3 > Task 1 (and Task 2)
        if (deadlines[0] < deadlines[2])
        {
            vTaskPrioritySet(T1, 3);
            vTaskPrioritySet(T2, 2);
            vTaskPrioritySet(T3, 1);
            return;
        }
        // Task 1 == Task 2 ; Task 1 (and Task 2) > Task 3
        else if (deadlines[0] > deadlines[2])
        {
            vTaskPrioritySet(T3, 3);
            vTaskPrioritySet(T1, 2);
            vTaskPrioritySet(T2, 1);
            return;
        }
    }

    // Task 1 > Task 3 > Task 2
    if ((deadlines[0] > deadlines[2]) && (deadlines[2] > deadlines[1])) 
    {
        vTaskPrioritySet(T2, 3);
        vTaskPrioritySet(T3, 2);
        vTaskPrioritySet(T1, 1);
        return;
    }
    // Task 3 > Task 1 > Task 2
    else if ((deadlines[0] < deadlines[2]) && (deadlines[0] < deadlines[2]))
    {
        vTaskPrioritySet(T2, 3);
        vTaskPrioritySet(T1, 2);
        vTaskPrioritySet(T3, 1);
        return;
    }
    // Task 1 == Task 3
    else if (deadlines[0] == deadlines[2])
    {
        // Task 1 == Task 3 ; Task 1 (and Task 3) > Task 2
        if (deadlines[0] > deadlines[1])
        {
            vTaskPrioritySet(T2, 3);
            vTaskPrioritySet(T1, 2);
            vTaskPrioritySet(T3, 1); 
            return;
        }
        // Task 1 == Task 3 ; Task 2 > Task 1 (and Task 3)
        else if (deadlines[0] < deadlines[1])
        {
            vTaskPrioritySet(T1, 3);
            vTaskPrioritySet(T3, 2);
            vTaskPrioritySet(T2, 1);
            return;
        }
    }
}

void execute_task(int taskNumber)
{
    int execution_time;
    long count;
    
    switch(taskNumber)
    {
        case 0:
            execution_time = TASK1_EXECUTION_TIME;
        break;
        case 1:
            execution_time = TASK2_EXECUTION_TIME;
        break;
        case 2:
            execution_time = TASK3_EXECUTION_TIME;
        break;
    }
    
    for (count = 0 ; count < (execution_time * MS_TO_S_MAGIC_NUMBER); count++)
    {
        // Execute task for execution_time seconds
    }
}

/*-----------------------------------------------------------*/

static void edfTask1( void *pvParameters )
{
    portTickType xRestartTime = 0;
    const portTickType xPeriod = TASK1_PERIOD * S_TO_MS;

    for( ;; )
    {
        edfScheduler();
        execute_task(0);
        vTaskDelayUntil(&xRestartTime, xPeriod);
        deadlines[0] += TASK1_PERIOD;
        edfScheduler();
    }
}

/*-----------------------------------------------------------*/

static void edfTask2( void *pvParameters )
{
    portTickType xRestartTime = 0;
    const portTickType xPeriod = TASK2_PERIOD * S_TO_MS;

    for( ;; )
    {
        edfScheduler();
        execute_task(1);
        vTaskDelayUntil(&xRestartTime, xPeriod);
        deadlines[1] += TASK2_PERIOD;
        edfScheduler();
    }
}

/*-----------------------------------------------------------*/
static void edfTask3( void *pvParameters )
{
    portTickType xRestartTime = 0;
    const portTickType xPeriod = TASK3_PERIOD * S_TO_MS;

    for( ;; )
    {
        edfScheduler();
        execute_task(2);
        vTaskDelayUntil(&xRestartTime, xPeriod);
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
    xTaskCreate( edfTask1, ( signed char * ) "T1", configMINIMAL_STACK_SIZE, NULL, 3, &T1 );
    xTaskCreate( edfTask2, ( signed char * ) "T2", configMINIMAL_STACK_SIZE, NULL, 2, &T2 );
    xTaskCreate( edfTask3, ( signed char * ) "T3", configMINIMAL_STACK_SIZE, NULL, 1, &T3 );

    // Start the FreeRTOS Default Scheduler
    vTaskStartScheduler();

    // Run indefinately
    while(1){}
    /*-----------------------------------------------------------*/
}