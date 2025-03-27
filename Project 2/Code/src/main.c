// Tay Munro V00965447
// Jett Lam  V00959283

// ##########################################################################
// 			IMPORTS GLOBAL VARIABLES AND FUNCTION DECLARATIONS
// ##########################################################################

/* Standard includes. */
#include "TaskMonitor.h"
#include "TaskGenerator.h"
#include "TaskScheduler.h"
#include "setup.h"

/* Kernel includes. */
#include "stm32f4xx.h"
#include "../FreeRTOS_Source/include/FreeRTOS.h"
#include "../FreeRTOS_Source/include/queue.h"
#include "../FreeRTOS_Source/include/semphr.h"
#include "../FreeRTOS_Source/include/task.h"
#include "../FreeRTOS_Source/include/timers.h"

#define TASK_SCHEDULER_PRIORITY  configMAX_PRIORITIES-2
#define TASK_MONITOR_PRIORITY    configMAX_PRIORITIES-2

void test_bench_init(int test_bench_number);


// ##########################################################################
// 								MAIN FUNCTION
// ##########################################################################

int main(void)
{
    printf("Starting\n");

	test_bench_init(TEST_BENCH);

	for (int i = 0; i < 3; i++) {
		PREEMPTED[i] = 0;
	}
    
    xQueue_Tasks = xQueueCreate(10, sizeof(dd_task));
    xQueue_Completed = xQueueCreate(10, sizeof(uint32_t));

    /* Create the Task Generator, which periodically creates DD-Tasks */
    xTaskCreate(DDScheduler, "Scheduler", configMINIMAL_STACK_SIZE, NULL, TASK_SCHEDULER_PRIORITY, NULL);
    
    /* Create the Task Monitor, which reports system status periodically */
    if (MONITOR_OR_DEBUG == 0) {
    	xTaskCreate(vTaskMonitor, "Monitor", configMINIMAL_STACK_SIZE, NULL, TASK_MONITOR_PRIORITY, NULL);
    }
    
    xTimer_Task1 = xTimerCreate("Timer1", 1, pdFALSE, (void *)0, Task1GenCallback);
    xTimer_Task2 = xTimerCreate("Timer2", 1, pdFALSE, (void *)0, Task2GenCallback);
    xTimer_Task3 = xTimerCreate("Timer3", 1, pdFALSE, (void *)0, Task3GenCallback);

    xTimerStart(xTimer_Task1, 0);
    xTimerStart(xTimer_Task2, 0);
    xTimerStart(xTimer_Task3, 0);
    
    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler();
    
    return 0;
}

void test_bench_init(int test_bench_number)
{
	if(test_bench_number == 1){
		ExecutionTime1 = 95;
		ExecutionTime2 = 150;
		ExecutionTime3 = 250;
		Period1 = 500;
		Period2 = 500;
		Period3 = 750;
	}
	else if(test_bench_number == 2){
		ExecutionTime1 = 95;
		ExecutionTime2 = 150;
		ExecutionTime3 = 250;
		Period1 = 250;
		Period2 = 500;
		Period3 = 750;
	}
	else if(test_bench_number == 3){
		ExecutionTime1 = 100;
		ExecutionTime2 = 200;
		ExecutionTime3 = 200;
		Period1 = 500;
		Period2 = 500;
		Period3 = 500;
	}

}


// ##########################################################################
// 						ERROR HANDLING FUNCTIONS
// ##########################################################################

void vApplicationMallocFailedHook( void )
{
	/* The malloc failed hook is enabled by setting
	configUSE_MALLOC_FAILED_HOOK to 1 in FreeRTOSConfig.h.

	Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
	for( ;; );
}

/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( xTaskHandle pxTask, signed char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected.  pxCurrentTCB can be
	inspected in the debugger if the task name passed into this function is
	corrupt. */
	for( ;; );
}

/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
volatile size_t xFreeStackSpace;

	/* The idle task hook is enabled by setting configUSE_IDLE_HOOK to 1 in
	FreeRTOSConfig.h.

	This function is called on each cycle of the idle task.  In this case it
	does nothing useful, other than report the amount of FreeRTOS heap that
	remains unallocated. */
	xFreeStackSpace = xPortGetFreeHeapSize();

	if( xFreeStackSpace > 100 )
	{
		/* By now, the kernel has allocated everything it is going to, so
		if there is a lot of heap remaining unallocated then
		the value of configTOTAL_HEAP_SIZE in FreeRTOSConfig.h can be
		reduced accordingly. */
	}
}

/*-----------------------------------------------------------*/

static void prvSetupHardware( void )
{
	/* Ensure all priority bits are assigned as preemption priority bits.
	http://www.freertos.org/RTOS-Cortex-M3-M4.html */
	NVIC_SetPriorityGrouping( 0 );

	/* TODO: Setup the clocks, etc. here, if they were not configured before
	main() was called. */
}
