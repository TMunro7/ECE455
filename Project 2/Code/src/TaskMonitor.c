#include "TaskMonitor.h"
#include "TaskScheduler.h"  // This header should declare the get_* functions and dd_task_list structure
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

/* 
 * Helper function to count the number of DD-Tasks in a linked list.
 * It iterates through the list until it reaches a NULL pointer.
 */
static uint32_t count_tasks(dd_task_list *list)
{
    uint32_t count = 0;
    dd_task_list *current = list;
    while (current != NULL)
    {
        count++;
        current = current->next_task;
    }
    return count;
}

/*
 * The Monitor Task.
 * Periodically queries the DDS for the number of active, completed, and overdue DD-Tasks,
 * and then prints out these counts.
 */
void vTaskMonitor(void *pvParameters)
{
    /* Suppress compiler warning if pvParameters is not used */
    (void)pvParameters;
    
    while (1)
    {
        /* Retrieve the DD-Task lists from the scheduler */
        dd_task_list *activeList   = get_active_dd_task_list();
        dd_task_list *completeList = get_complete_dd_task_list();
        dd_task_list *overdueList  = get_overdue_dd_task_list();

        /* Count the tasks in each list */
        uint32_t activeCount   = count_tasks(activeList);
        uint32_t completeCount = count_tasks(completeList);
        uint32_t overdueCount  = count_tasks(overdueList);

        /* Report the current status to the console */
        printf("Monitor: Active: %lu, Completed: %lu, Overdue: %lu\n",
               (unsigned long)activeCount,
               (unsigned long)completeCount,
               (unsigned long)overdueCount);

        /* Delay before the next monitoring period (e.g., 1000ms) */
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
