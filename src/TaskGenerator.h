#ifndef TASK_GENERATOR_H
#define TASK_GENERATOR_H

#include "FreeRTOS.h"
#include "task.h"
#include "setup.h"
#include "../FreeRTOS_Source/include/timers.h"

/* Task Generator Task function prototype */
void Task1(void *pvParameters);
void Task2(void *pvParameters);
void Task3(void *pvParameters);

void Task1GenCallback(TimerHandle_t pxTimer);
void Task2GenCallback(TimerHandle_t pxTimer);
void Task3GenCallback(TimerHandle_t pxTimer);

#endif /* TASK_GENERATOR_H */
