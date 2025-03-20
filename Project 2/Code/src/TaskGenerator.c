#include "TaskGenerator.h"
#include "TaskScheduler.h"  // Assumes this header declares release_dd_task and related types.
#include "FreeRTOS.h"
#include "task.h"

/* Constants for task generation */
#define TASK_GENERATION_INTERVAL_MS 500    // Time between generating tasks (in ms)
#define DEADLINE_OFFSET_TICKS       200    // Offset (in ticks) added to current time for deadline

/*
 * vTaskGenerator - Task function that periodically generates DD-Tasks.
 *
 * This function simulates the generation of Deadline-Driven Tasks (DD-Tasks)
 * by calling release_dd_task with a dummy FreeRTOS task handle (NULL in this example),
 * a fixed task type, an incrementing task ID, and an absolute deadline based on
 * the current tick count plus an offset.
 */
void vTaskGenerator(void *pvParameters)
{
    (void) pvParameters;
    
    uint32_t dd_task_counter = 0;
    
    while (1)
    {
        /* Get the current tick count */
        TickType_t current_tick = xTaskGetTickCount();
        
        /* Compute an absolute deadline offset from now */
        TickType_t deadline = current_tick + DEADLINE_OFFSET_TICKS;
        
        /*
         * For this example, we assume all generated tasks are periodic.
         * If you need to alternate or determine the task type dynamically,
         * modify the second parameter accordingly.
         *
         * Here, we pass a dummy task handle (NULL) since the real
         * user-defined FreeRTOS task may be created elsewhere.
         */
        release_dd_task(NULL, PERIODIC, dd_task_counter, (uint32_t)deadline);
        
        /* Increment the DD-Task ID for the next task */
        dd_task_counter++;
        
        /* Wait for the next generation period */
        vTaskDelay(pdMS_TO_TICKS(TASK_GENERATION_INTERVAL_MS));
    }
}
