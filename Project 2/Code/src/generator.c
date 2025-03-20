#include "main.c"

void NewTask(void *pvParameters) {
    uint16_t task_exec;
    xQueuePeek(xQueue_Task_Exec, &task_exec, 5000);

    uint16_t task_num;
    xQueuePeek(xQueue_Task_Nums, &task_num, 5000);

    TickType_t start = xTaskGetTickCount();
    TickType_t cur;
    TickType_t elapsed = 0;
    TickType_t remaining = pdMS_TO_TICKS(task_exec);

    while (elapsed < remaining) {
        if (TASK_PREEMPTED[task_num - 1]) {
            start = xTaskGetTickCount();
            TASK_PREEMPTED[task_num - 1] = 0;
        }

        cur = xTaskGetTickCount();
        elapsed = cur - start;
    }

    delete_dd_task(task_num);
    vTaskDelete(NULL);
}


void TaskGeneratorCallback(TimerHandle_t pxTimer) {
    TaskHandle_t cur_task;
    xQueuePeek(xQueue_Tasks, &cur_task, 5000);
    
    TaskHandle_t task;
    xTaskCreate(cur_task, );
}