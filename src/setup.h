#include "../FreeRTOS_Source/include/queue.h"
#include "../FreeRTOS_Source/include/timers.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "stm32f4_discovery.h"

volatile uint32_t ExecutionTime1;
volatile uint32_t Period1;
volatile uint32_t ExecutionTime2;
volatile uint32_t Period2;
volatile uint32_t ExecutionTime3;
volatile uint32_t Period3;
volatile int PREEMPTED[3];

#define MONITOR_OR_DEBUG 1
#define TEST_BENCH 1

QueueHandle_t xQueue_Tasks, xQueue_Completed;
TimerHandle_t xTimer_Task1, xTimer_Task2, xTimer_Task3;
