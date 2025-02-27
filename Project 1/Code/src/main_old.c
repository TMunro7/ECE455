/*
    FreeRTOS V9.0.0 - Copyright (C) 2016 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wwrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

/*
FreeRTOS is a market leading RTOS from Real Time Engineers Ltd. that supports
31 architectures and receives 77500 downloads a year. It is professionally
developed, strictly quality controlled, robust, supported, and free to use in
commercial products without any requirement to expose your proprietary source
code.

This simple FreeRTOS demo does not make use of any IO ports, so will execute on
any Cortex-M3 of Cortex-M4 hardware.  Look for TODO markers in the code for
locations that may require tailoring to, for example, include a manufacturer
specific header file.

This is a starter project, so only a subset of the RTOS features are
demonstrated.  Ample source comments are provided, along with web links to
relevant pages on the http://www.FreeRTOS.org site.

Here is a description of the project's functionality:

The main() Function:
main() creates the tasks and software timers described in this section, before
starting the scheduler.

The Queue Send Task:
The queue send task is implemented by the prvQueueSendTask() function.
The task uses the FreeRTOS vTaskDelayUntil() and xQueueSend() API functions to
periodically send the number 100 on a queue.  The period is set to 200ms.  See
the comments in the function for more details.
http://www.freertos.org/vtaskdelayuntil.html
http://www.freertos.org/a00117.html

The Queue Receive Task:
The queue receive task is implemented by the prvQueueReceiveTask() function.
The task uses the FreeRTOS xQueueReceive() API function to receive values from
a queue.  The values received are those sent by the queue send task.  The queue
receive task increments the ulCountOfItemsReceivedOnQueue variable each time it
receives the value 100.  Therefore, as values are sent to the queue every 200ms,
the value of ulCountOfItemsReceivedOnQueue will increase by 5 every second.
http://www.freertos.org/a00118.html

An example software timer:
A software timer is created with an auto reloading period of 1000ms.  The
timer's callback function increments the ulCountOfTimerCallbackExecutions
variable each time it is called.  Therefore the value of
ulCountOfTimerCallbackExecutions will count seconds.
http://www.freertos.org/RTOS-software-timer.html

The FreeRTOS RTOS tick hook (or callback) function:
The tick hook function executes in the context of the FreeRTOS tick interrupt.
The function 'gives' a semaphore every 500th time it executes.  The semaphore
is used to synchronise with the event semaphore task, which is described next.

The event semaphore task:
The event semaphore task uses the FreeRTOS xSemaphoreTake() API function to
wait for the semaphore that is given by the RTOS tick hook function.  The task
increments the ulCountOfReceivedSemaphores variable each time the semaphore is
received.  As the semaphore is given every 500ms (assuming a tick frequency of
1KHz), the value of ulCountOfReceivedSemaphores will increase by 2 each second.

The idle hook (or callback) function:
The idle hook function queries the amount of free FreeRTOS heap space available.
See vApplicationIdleHook().

The malloc failed and stack overflow hook (or callback) functions:
These two hook functions are provided as examples, but do not contain any
functionality.
*/

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



/*-----------------------------------------------------------*/
#define TRAFFIC_LIGHT_QUEUE_MAX		1
#define RED_LED						GPIO_Pin_0
#define YELLOW_LED					GPIO_Pin_1
#define GREEN_LED					GPIO_Pin_2
#define POT							GPIO_Pin_3
#define SHIFT_DATA					GPIO_Pin_6
#define SHIFT_CLOCK					GPIO_Pin_7
#define SHIFT_RESET					GPIO_Pin_8
#define MIN_TIME					3000
#define MAX_TIME					9000

/*
 * The queue send and receive tasks as described in the comments at the top of
 * this file.
 */
static void Traffic_Light_Task(void *pvParameters);
static void Traffic_Flow_Task(void *pvParameters);
static void System_Display_Task(void *pvParameters);
static void Traffic_Generator_Task(void *pvParameters);

static void GPIO_INIT();
static void ADC_INIT();
static void Turn_On_Green(uint16_t GLED);
static void Turn_On_Red(uint16_t RLED);
static void Turn_On_Yellow(uint16_t YLED);
static uint16_t Get_ADC_Val();
static uint16_t Time_Scale(uint16_t led);



xQueueHandle xQueue_Light = 0;
xQueueHandle xQueue_Flow = 0;
xQueueHandle xQueue_Cars = 0;

/*-----------------------------------------------------------*/

int main(void)
{

	// Initialize LEDs
	GPIO_INIT();
	ADC_INIT();

	// Create the queue used by the queue send and queue receive tasks
	xQueue_Light = xQueueCreate(TRAFFIC_LIGHT_QUEUE_MAX, sizeof( uint16_t ) );
	xQueue_Flow = xQueueCreate(TRAFFIC_LIGHT_QUEUE_MAX, sizeof( uint16_t ) );
	xQueue_Cars = xQueueCreate(TRAFFIC_LIGHT_QUEUE_MAX, sizeof( uint16_t ) );

	// Add to the registry, for the benefit of kernel aware debugging
	vQueueAddToRegistry( xQueue_Light, "LightQueue" );
	vQueueAddToRegistry( xQueue_Flow, "FlowQueue" );
	vQueueAddToRegistry( xQueue_Cars, "CarQueue" );

	xTaskCreate( Traffic_Light_Task, "Traffic_Light", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	xTaskCreate( Traffic_Flow_Task, "Traffic_Flow", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	xTaskCreate( System_Display_Task, "System_Display", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	xTaskCreate( Traffic_Generator_Task, "Traffic_Generator", configMINIMAL_STACK_SIZE, NULL, 1, NULL);


	// Start the tasks and timer running
	vTaskStartScheduler();

	for ( ;; );

	return 0;
}

/*-----------------------------------------------------------*/

static void GPIO_INIT()
{
	// First enable the GPIOC clock
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	// Setup GPIO structure for traffic lights
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin =		RED_LED | YELLOW_LED | GREEN_LED;
	GPIO_InitStruct.GPIO_Mode =		GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Speed = 	GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStruct);

	// Setup GPIO structure for pot
	GPIO_InitStruct.GPIO_Pin =		POT;
	GPIO_InitStruct.GPIO_Mode =		GPIO_Mode_AN;
	GPIO_InitStruct.GPIO_PuPd =		GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStruct);

	// Setup shift registers for traffic flow
	GPIO_InitStruct.GPIO_Pin =		SHIFT_DATA | SHIFT_CLOCK | SHIFT_RESET;
	GPIO_InitStruct.GPIO_Mode =		GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Speed =	GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStruct);
}

/*-----------------------------------------------------------*/

static void ADC_INIT()
{
	// First enable the ADC clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	// Setup ADC structure
	ADC_InitTypeDef ADC_InitStruct;
	ADC_InitStruct.ADC_ContinuousConvMode =		DISABLE;
	ADC_InitStruct.ADC_DataAlign =				ADC_DataAlign_Right;
	ADC_InitStruct.ADC_Resolution =				ADC_Resolution_12b;
	ADC_InitStruct.ADC_ScanConvMode =			DISABLE;
	ADC_InitStruct.ADC_ExternalTrigConv =		DISABLE;
	ADC_InitStruct.ADC_ExternalTrigConvEdge =	DISABLE;

	// Apply the setup to the ADC and enable
	ADC_Init(ADC1, &ADC_InitStruct);
	ADC_Cmd(ADC1, ENABLE);

	// Set the channel
	ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 1, ADC_SampleTime_144Cycles);
}

/*-----------------------------------------------------------*/

static void Traffic_Light_Task( void *pvParameters )
{
	uint16_t GLED = GREEN_LED;
	uint16_t YLED = YELLOW_LED;
	uint16_t RLED = RED_LED;

	// Initial reset to make sure we are always starting on neutral
	GPIO_SetBits(GPIOC, RESET);

	while(1)
	{
		Turn_On_Green(GLED);
		vTaskDelay(pdMS_TO_TICKS(Time_Scale(GLED)));

		Turn_On_Yellow(YLED);
		vTaskDelay(pdMS_TO_TICKS(MIN_TIME));

		Turn_On_Red(RLED);
		vTaskDelay(pdMS_TO_TICKS(Time_Scale(RLED)));
	}
}

static void Turn_On_Red( uint16_t RLED )
{
	xQueueOverwrite(xQueue_Light, &RLED);
	GPIO_ResetBits(GPIOC, YELLOW_LED);
	GPIO_SetBits(GPIOC, RLED);
}

static void Turn_On_Yellow( uint16_t YLED )
{
	xQueueOverwrite(xQueue_Light, &YLED);
	GPIO_ResetBits(GPIOC, GREEN_LED);
	GPIO_SetBits(GPIOC, YLED);
}

static void Turn_On_Green( uint16_t GLED )
{
	xQueueOverwrite(xQueue_Light, &GLED);
	GPIO_ResetBits(GPIOC, RED_LED);
	GPIO_SetBits(GPIOC, GLED);
}

static uint16_t Time_Scale( uint16_t led )
{
	uint16_t pot_val = Get_ADC_Val();
	if (led == GREEN_LED){
		return pdMS_TO_TICKS(MIN_TIME) + ((pot_val * (pdMS_TO_TICKS(6000))) / 4095);
	}
	else {
		return pdMS_TO_TICKS(MAX_TIME) - ((pot_val * (pdMS_TO_TICKS(6000))) / 4095);
	}
}

/*-----------------------------------------------------------*/

static void System_Display_Task( void *pvParameters )
{
	// Create a list of 19 cars.
	// The first 8 cars (cars 0 to 7) are before the stop line and will stop on red.
	// The next 11 cars (8 to 18) are after the stop line and don't get affected by the lights.
	int car_traffic[19] = {0};
	int queued_cars = 0;
	uint16_t traffic_light_status;
	GPIO_SetBits(GPIOC, SHIFT_RESET);

	// Start loop of program running
	while(1){
		// Get colour of the traffic light with 5000 ms timeout
		xQueuePeek(xQueue_Light, &traffic_light_status, 5000);
		for (int i=19; i>0; i--){
			if(car_traffic[i]){
				// If there is a car in this spot of the array, turn on corresponding light
				GPIO_SetBits(GPIOC, SHIFT_DATA);
			} else {
				// Else turn off the corresponding light
				GPIO_ResetBits(GPIOC, SHIFT_DATA);
			}
			// Move to the next spot in the shift register
			GPIO_SetBits(GPIOC, SHIFT_CLOCK);
			GPIO_ResetBits(GPIOC, SHIFT_CLOCK);
		}
		if(traffic_light_status == GREEN_LED){
			// If the light is green, shift the array one unit to the right for each tick
			for(int i = 19; i>0; i--){
				car_traffic[i] = car_traffic[i-1];
				//car_traffic[i-1] = 0;
			}
		} else {
			// Else, shift cars according to what needs to be done on a red/yellow light
			for (int i=8; i>0; i--){
				// First 8 cars (0 to 7) stop before the lights
				if(!(car_traffic[i])){
					// If next car is empty, shift current car to next position and set current position to empty
					car_traffic[i] = car_traffic[i-1];
					car_traffic[i-1] = 0;
				}
			}
			for (int i=19; i>9; i--){
				// Move cars normally after the lights.
				car_traffic[i] = car_traffic[i-1];
				car_traffic[i-1] = 0;
			}

		}
		// Add car to the road.
		/*if(xQueuePeek(xQueue_Cars, &queued_cars, 5000) == pdPASS)
		{
			if(queued_cars)
			{
				car_traffic[0] = 1;
				xQueueOverwrite(xQueue_Cars, 0);
			}
		}*/


//		xQueuePeek(xQueue_Cars, &car_traffic[0], 5000);
//		xQueueOverwrite(xQueue_Cars, 0);
		//car_traffic[0] = 1;
		uint16_t car = 0;
		BaseType_t cars_status = xQueueReceive(xQueue_Cars, &car, pdMS_TO_TICKS(100));
		car_traffic[0] = 0;
		if(cars_status == pdTRUE)
		{
			if(car == 1)
			{
				car_traffic[0] = 1;
			}
		}
		vTaskDelay(pdMS_TO_TICKS(500));
	}

}

static void Traffic_Flow_Task( void *pvParameters )
{
	for ( ;; )
	{
		uint16_t flow_rate = (uint16_t)(Time_Scale(RED_LED)/1000) - 3;
		xQueueOverwrite(xQueue_Flow, &flow_rate);
	}
}

static void Traffic_Generator_Task( void *pvParameters )
{
	uint16_t flow;
	uint16_t car = 1;
	BaseType_t status;

	for ( ;; )
	{
		status = xQueuePeek(xQueue_Flow, &flow, 5000);

		if(status == pdPASS)
		{
			xQueueOverwrite(xQueue_Cars, &car);
			vTaskDelay(pdMS_TO_TICKS(500 * flow));
		}
	}
}

static uint16_t Get_ADC_Val()
{
	ADC_SoftwareStartConv(ADC1);
	while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
	return ADC_GetConversionValue(ADC1);
}

/*-----------------------------------------------------------*/

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