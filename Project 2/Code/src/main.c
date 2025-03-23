// Tay Munro V00965447
// Jett Lam  V00959283

// ##########################################################################
// 			IMPORTS GLOBAL VARIABLES AND FUNCTION DECLARATIONS
// ##########################################################################

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include "stm32f4_discovery.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_gpio.h"

/* Kernel includes. */
#include "stm32f4xx.h"
#include "../FreeRTOS_Source/include/FreeRTOS.h"
#include "../FreeRTOS_Source/include/queue.h"
#include "../FreeRTOS_Source/include/semphr.h"
#include "../FreeRTOS_Source/include/task.h"
#include "../FreeRTOS_Source/include/timers.h"

#define TASK_GENERATOR_PRIORITY  (tskIDLE_PRIORITY + 1)
#define TASK_MONITOR_PRIORITY    (tskIDLE_PRIORITY + 1)
#define TEST_BENCH_NUMBER = 1 // Change this to the number of the test bench you are running

volatile uint32_t ExecutionTime1;
volatile uint32_t Period1;
volatile uint32_t ExecutionTime2;
volatile uint32_t Perioid2;
volatile uint32_t ExecutionTime3;
volatile uint32_t Period;


// ##########################################################################
// 								MAIN FUNCTION
// ##########################################################################

int main(void)
{
    
	test_bench_init(TEST_BENCH_NUMBER);
    /* Initialize the DDS scheduler module */
    init_dd_scheduler();
    
    /* Create the Task Generator, which periodically creates DD-Tasks */
    xTaskCreate(vTaskGenerator, "TaskGen", configMINIMAL_STACK_SIZE, NULL, TASK_GENERATOR_PRIORITY, NULL);
    
    /* Create the Task Monitor, which reports system status periodically */
    xTaskCreate(vTaskMonitor, "Monitor", configMINIMAL_STACK_SIZE, NULL, TASK_MONITOR_PRIORITY, NULL);
    
    /* Create any additional user-defined tasks here */
    
    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler();
    
    /* Should never reach here since vTaskStartScheduler only returns on error */
    for(;;);
    
    return 0;
}

void test_bench_init(int test_bench_number)
{
	if(test_bench_number == 1){
		ExecutionTime1 = 95;
		ExecutionTime2 = 150;
		ExecutionTime3 250;
		Period1 = 500;
		Perioid2 = 500;
		Period = 750;
	}
	if(test_bench_number == 2){
		ExecutionTime1 = 95;
		ExecutionTime2 = 150;
		ExecutionTime3 = 250;
		Period1 = 250;
		Perioid2 = 500;
		Period = 750;
	}
	if(test_bench_number == 3){
		ExecutionTime1 = 100;
		ExecutionTime2 = 200;
		ExecutionTime3 = 200;
		Period1 = 500;
		Perioid2 = 500;
		Period = 500;
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
