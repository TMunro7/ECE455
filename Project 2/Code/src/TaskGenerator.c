#include "TaskGenerator.h"
#include "TaskScheduler.h"  // Assumes this header declares release_dd_task and related types.
#include "FreeRTOS.h"
#include "task.h"
#include "setup.h"

void Task1(void *pvParameters) {
    TickType_t start = xTaskGetTickCount();
    TickType_t cur;
    TickType_t elapsed = 0;
    TickType_t remaining = pdMS_TO_TICKS(ExecutionTime1);

    while (elapsed < remaining) {
        if (PREEMPTED[0]) {
            start = xTaskGetTickCount();
            PREEMPTED[0] = 0;
        }

        cur = xTaskGetTickCount();
        elapsed = cur - start;
    }

    delete_dd_task(1);
    vTaskDelete(NULL);
}


void Task2(void *pvParameters) {
    TickType_t start = xTaskGetTickCount();
    TickType_t cur;
    TickType_t elapsed = 0;
    TickType_t remaining = pdMS_TO_TICKS(ExecutionTime2);

    while (elapsed < remaining) {
        if (PREEMPTED[1]) {
            start = xTaskGetTickCount();
            PREEMPTED[1] = 0;
        }

        cur = xTaskGetTickCount();
        elapsed = cur - start;
    }

    delete_dd_task(2);
    vTaskDelete(NULL);
}


void Task3(void *pvParameters) {
    TickType_t start = xTaskGetTickCount();
    TickType_t cur;
    TickType_t elapsed = 0;
    TickType_t remaining = pdMS_TO_TICKS(ExecutionTime3);

    while (elapsed < remaining) {
        if (PREEMPTED[2]) {
            start = xTaskGetTickCount();
            PREEMPTED[2] = 0;
        }

        cur = xTaskGetTickCount();
        elapsed = cur - start;
    }

    delete_dd_task(3);
    vTaskDelete(NULL);
}


void Task1GenCallback(TimerHandle_t pxTimer) {
    TaskHandle_t task;
    xTaskCreate(Task1, "Task 1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &task);

    if (task != NULL) {
        TickType_t cur = xTaskGetTickCount();
        create_dd_task(task, PERIODIC, 1, cur, cur+pdMS_TO_TICKS(Period1));
    }

    xTimerChangePeriod(pxTimer, pdMS_TO_TICKS(Period1), 0);
    xTimerStart(pxTimer, 0);
}


void Task2GenCallback(TimerHandle_t pxTimer) {
    TaskHandle_t task;
    xTaskCreate(Task2, "Task 2", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &task);

    if (task != NULL) {
        TickType_t cur = xTaskGetTickCount();
        create_dd_task(task, PERIODIC, 2, cur, cur+pdMS_TO_TICKS(Period2));
    }

    xTimerChangePeriod(pxTimer, pdMS_TO_TICKS(Period2), 0);
    xTimerStart(pxTimer, 0);
}


void Task3GenCallback(TimerHandle_t pxTimer) {
    TaskHandle_t task;
    xTaskCreate(Task3, "Task 3", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &task);

    if (task != NULL) {
        TickType_t cur = xTaskGetTickCount();
        create_dd_task(task, PERIODIC, 3, cur, cur+pdMS_TO_TICKS(Period3));
    }

    xTimerChangePeriod(pxTimer, pdMS_TO_TICKS(Period3), 0);
    xTimerStart(pxTimer, 0);
}
