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

// ##########################################################################
// 								MAIN FUNCTION
// ##########################################################################

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

// ##########################################################################
// 					INITIALIZATION FUNCTIONS AND TASKS
// ##########################################################################
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

// ##########################################################################
// 					TRAFFIC LIGHT TASK AND FUNCTIONS
// ##########################################################################

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

// ##########################################################################
// 					SYSTEM DISPLAY TASK AND FUNCTIONS
// ##########################################################################

static void System_Display_Task( void *pvParameters )
{
	// Create a list of 19 cars.
	// The first 8 cars (cars 18 to 11) are before the stop line and will stop on red.
	// The next 11 cars (10 to 0) are after the stop line and don't get affected by the lights.
	int car_traffic[19] = {0};
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
				// First 8 cars (18 to 11) stop before the lights
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
		// Check if a car is coming and add it to the array
		uint16_t car = 0;
		xQueueReceive(xQueue_Cars, &car, pdMS_TO_TICKS(100));
		car_traffic[0] = 0;
			if(car == 1)
			{
				car_traffic[0] = 1;
			}
		
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}

// ##########################################################################
// 					TRAFFIC FLOW TASK AND FUNCTIONS
// ##########################################################################

static void Traffic_Flow_Task( void *pvParameters )
{
	while(1)
	{
		// Use the time scale function to determine the flow rate
		// If the light is red for the maximum amount of time, flow is a gap of 5 between cars
		// If the light is red for the minimum amount of time, flow is a gap of 0 between cars
		uint16_t flow_rate = (uint16_t)(Time_Scale(RED_LED)/1000) - 3;
		xQueueOverwrite(xQueue_Flow, &flow_rate);
	}
}

static uint16_t Get_ADC_Val()
{
	ADC_SoftwareStartConv(ADC1);
	while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
	return ADC_GetConversionValue(ADC1);
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

// ##########################################################################
// 					TRAFFIC GENERATOR TASK AND FUNCTIONS
// ##########################################################################
static void Traffic_Generator_Task( void *pvParameters )
{
	uint16_t flow;
	uint16_t car = 1;

	while(1)
	{
		xQueuePeek(xQueue_Flow, &flow, 5000);
		xQueueOverwrite(xQueue_Cars, &car);
		vTaskDelay(pdMS_TO_TICKS(500 * flow));
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
