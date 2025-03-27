#include "TaskMonitor.h"
#include "TaskScheduler.h"
#include "FreeRTOS.h"
#include "task.h"
#include "setup.h"

volatile int EXPECTED_OVERDUE;
volatile int EXPECTED_COMPLETE;
volatile int EXPECTED_ACTIVE;
volatile dd_task_list *active_task_list;
volatile dd_task_list *completed_task_list;
volatile dd_task_list *overdue_task_list;

/*
 * The Monitor Task.
 * Periodically queries the DDS for the number of active, completed, and overdue DD-Tasks,
 * and then prints out these counts.
 */
void vTaskMonitor(void *pvParameters)
{
 printf("Task Monitor Started\n");
    for(;;) {
        active_task_list = get_active_dd_task_list();
        completed_task_list = get_complete_dd_task_list();
        overdue_task_list = get_overdue_dd_task_list();
        EXPECTED_OVERDUE = 0;
        EXPECTED_COMPLETE = 0;
        EXPECTED_ACTIVE = 0;

        dd_task_list *temp = overdue_task_list;
        while(temp != NULL) {
            EXPECTED_OVERDUE++;
            temp = temp->next_task;
        }

        temp = completed_task_list;
        while(temp != NULL) {
            EXPECTED_COMPLETE++;
            temp = temp->next_task;
        }

        temp = active_task_list;
        while (temp != NULL)
        {
            EXPECTED_ACTIVE++;
            temp = temp->next_task;
        }

        printf("----------------------------------------------\n");
        printf("Active: %d, Overdue: %d, Complete: %d\n", EXPECTED_ACTIVE, EXPECTED_OVERDUE, EXPECTED_COMPLETE);
        printf("----------------------------------------------\n");
        vTaskDelay(pdMS_TO_TICKS(1505));
    }
}
