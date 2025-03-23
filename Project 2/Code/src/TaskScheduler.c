#include "TaskScheduler.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>

volatile dd_task_list *active_tasks;
volatile dd_task_list *completed_tasks;
volatile dd_task_list *overdue_tasks;

volatile dd_task *cur_task = NULL;
volatile uint32_t cur_tick;

volatile int BENCH_1_RELEASE_VALUES[3][13] = {
    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000},
    {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000},
    {0, 750, 1500, 2250, 3000, 3750, 4500, 5250, 6000}
};
volatile int BENCH_1_COMPLETED_VALUES[3][13] = {
    {95, 595, 1095, 1595, 2095, 2595, 3095, 3595, 4095, 4595, 5095, 5595, 6095},
    {245, 745, 1245, 1745, 2245, 2745, 3245, 3745, 4245, 4745, 5245, 5745, 6245},
    {495, 1000, 1995, 2500, 3495, 4000, 4995, 5500, 6495}
};

volatile int TASK_RELEASE_COUNT[3] = {0, 0, 0};
volatile int TASK_COMPLETED_COUNT[3] = {0, 0, 0};
volatile int EVENT_NUM = 1;

void DDScheduler(void *pvParameters) {
    if (MONITOR_OR_DEBUG == 1) {
        printf("Begin scheduling.\n");
    }

    while (1) {
        dd_task *new_task;
        uint32_t *completed_task_id;
        cur_tick = xTaskGetTickCount();

        if (xQueueReceive(xQueue_Completed, &completed_task_id, 0) == pdTRUE && completed_task_id != NULL && cur_task != NULL) {
            if (completed_task_id == cur_task->task_id) {
                dd_task *removed_task = removed_from_list((volatile dd_task_list **) &active_tasks, completed_task_id);
                removed_tasks->completion_time = cur_tick;
                vTaskDelete(cur_task->t_handle);

                if (MONITOR_OR_DEBUG == 1) {
                    if (TEST_BENCH == 1) {
                        printf("%d Task %d completed: %d Ex: %d\n", EVENT_NUMBER, completed_task_id, removed_task->completion_time, BENCH_1_COMPLETED_VALUES[removed_task->task_id - 1][TASK_COMPLETED_COUNT[removed_task->task_id - 1]]);
                        TASK_COMPLETED_COUNT[removed_task->task_id - 1] += 1;
                    } else {
                        printf("%d Task %d completed: %d\n", EVENT_NUMBER, completed_task_id, removed_task->completion_time);
                    }
                }

                push_to_list((volatile dd_task_list **) &completed_tasks, removed_task);
                EVENT_NUM += 1;
                cur_task = NULL;
            
            } else {
                printf("Error: task scheduler failed at line 58");
            }
        }

        if (xQueueReceive(xQueue_Tasks, &new_task, 0) == pdTRUE && new_task != NULL) {
            if (MONITOR_OR_DEBUG == 1) {
                if (TEST_BENCH == 1) {
                    printf("%d Task %d released:  %d Ex: %d\n", EVENT_NUMBER, new_task->task_id, current_tick, TB_1_RELEASE_VALUES[new_task->task_id - 1][TASK_RELEASE_COUNT[new_task->task_id - 1]]);
					TASK_RELEASE_COUNT[new_task->task_id - 1] += 1;
				} else {
					printf("%d Task %d released:  %d\n", EVENT_NUMBER, new_task->task_id, current_tick);
				}
            }

            push_to_list_edf((volatile dd_task_list **) &active_tasks, new_task);
            EVENT_NUM += 1;
        }

        if (cur_task != NULL && cur_tick > cur_task->deadline) {
            if (MONITOR_OR_DEBUG == 1) {
                printf("Task %d OVERDUE: %d DL: %d\n", current_task->task_id, current_tick, current_task->absolute_deadline);
            }
            dd_task *removed_task = remove_from_list((volatile dd_task_list **) &active_tasks, cur_task->task_id);
            vTaskDelete(cur_task->t_handle);
            push_to_list((volatile dd_task_list **) &overdue_tasks, removed_task);
            EVENT_NUM += 1;
            cur_task = NULL
        }

        dd_task *first_task = NULL;
        if (active_tasks != NULL) {
            first_task = active_tasks->task;

            if (first_task != NULL && cur_task !- NULL && first_task->deadline < cur_task->deadline) {
                vTaskPrioritySet(cur_task->t_handle, tskIDLE_PRIORITY);
                PREEMPTED[cur_task->task_id - 1] = 1;
                cur_task = first_task;
                vTaskPrioritySet(frist_task->t_handle, configMAX_PRIORITIES - 2);
            }

            if (first_task != NULL && cur_task == NULL) {
                cur_task = first_task;
                vTaskPrioritySet(cur_task->t_handle, configMAX_PRIORITIES - 2);
            }
        }
    }
}