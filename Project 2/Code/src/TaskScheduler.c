#include "TaskScheduler.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "setup.h"

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


struct dd_task_list *get_active_dd_task_list(void) {
    return active_tasks;
}

struct dd_task_list *get_complete_dd_task_list(void) {
    return completed_tasks;
}

struct dd_task_list *get_overdue_dd_task_list(void) {
    return overdue_tasks;
}


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
                dd_task *removed_task = remove_from_list((volatile dd_task_list **) &active_tasks, completed_task_id);
                removed_task->completion = cur_tick;
                vTaskDelete(cur_task->t_handle);

                if (MONITOR_OR_DEBUG == 1) {
                    if (TEST_BENCH == 1) {
                        printf("%d Task %d completed: %d Ex: %d\n", EVENT_NUM, (int)completed_task_id, (int)removed_task->completion, BENCH_1_COMPLETED_VALUES[removed_task->task_id - 1][TASK_COMPLETED_COUNT[removed_task->task_id - 1]]);
                        TASK_COMPLETED_COUNT[removed_task->task_id - 1] += 1;
                    } else {
                        printf("%d Task %d completed: %d\n", EVENT_NUM, (int)completed_task_id, (int)removed_task->completion);
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
                    printf("%d Task %d released:  %d Ex: %d\n", EVENT_NUM, (int)new_task->task_id, (int)cur_tick, BENCH_1_RELEASE_VALUES[new_task->task_id - 1][TASK_RELEASE_COUNT[new_task->task_id - 1]]);
					TASK_RELEASE_COUNT[new_task->task_id - 1] += 1;
				} else {
					printf("%d Task %d released:  %d\n", EVENT_NUM, (int)new_task->task_id, (int)cur_tick);
				}
            }

            push_to_list_edf((volatile dd_task_list **) &active_tasks, new_task);
            EVENT_NUM += 1;
        }

        if (cur_task != NULL && cur_tick > cur_task->deadline) {
            if (MONITOR_OR_DEBUG == 1) {
                printf("Task %d OVERDUE: %d DL: %d\n", (int)cur_task->task_id, (int)cur_tick, (int)cur_task->deadline);
            }
            dd_task *removed_task = remove_from_list((volatile dd_task_list **) &active_tasks, cur_task->task_id);
            vTaskDelete(cur_task->t_handle);
            push_to_list((volatile dd_task_list **) &overdue_tasks, removed_task);
            EVENT_NUM += 1;
            cur_task = NULL;
        }

        dd_task *first_task = NULL;
        if (active_tasks != NULL) {
            first_task = active_tasks->task;

            if (first_task != NULL && cur_task != NULL && first_task->deadline < cur_task->deadline) {
                vTaskPrioritySet(cur_task->t_handle, tskIDLE_PRIORITY);
                PREEMPTED[cur_task->task_id - 1] = 1;
                cur_task = first_task;
                vTaskPrioritySet(first_task->t_handle, configMAX_PRIORITIES - 2);
            }

            if (first_task != NULL && cur_task == NULL) {
                cur_task = first_task;
                vTaskPrioritySet(cur_task->t_handle, configMAX_PRIORITIES - 2);
            }
        }
    }
}


void create_dd_task(TaskHandle_t t_handle, task_type type, uint32_t task_id, uint32_t release, uint32_t deadline) {
    dd_task *task = (dd_task *)pvPortMalloc(sizeof(dd_task));

    if (task == NULL) {
        printf("Failed to create task.\n");
        return;
    }

    task->t_handle = t_handle;
    task->type = type;
    task->task_id = task_id;
    task->release = pdMS_TO_TICKS(release);
    task->deadline = pdMS_TO_TICKS(deadline);
    task->completion = 0;

    xQueueSend(xQueue_Tasks, &task, 0);
}


void delete_dd_task(uint32_t task_id) {
    xQueueSend(xQueue_Completed, &task_id, 50);
}


void push_to_list(volatile dd_task_list **list, dd_task *new_task) {
    dd_task_list *new_node = (dd_task_list *)malloc(sizeof(dd_task_list));
    if (new_node == NULL) {
        return;
    }

    new_node->task = new_task;
    new_node->next_task = NULL;

    if (*list == NULL) {
        *list = new_node;
        return;
    }

    dd_task_list *cur = *list;
    while (cur->next_task != NULL) {
        cur = cur->next_task;
    }

    cur->next_task = new_node;
}


void push_to_list_edf(volatile dd_task_list **list, dd_task *new_task) {
    dd_task_list *new_node = (dd_task_list *)malloc(sizeof(dd_task_list));
    if (new_node == NULL) {
        return;
    }

    new_node->task = new_task;
    new_node->next_task = NULL;

    if (*list == NULL || (*list)->task->deadline > new_task->deadline) {
        new_node->next_task = *list;
        *list = new_node;
        return;
    }

    dd_task_list *cur = *list;
    while (cur->next_task != NULL) {
        dd_task_list *check = cur->next_task;
        if (check->task->deadline > new_task->deadline) {
            break;
        } else {
            cur = cur->next_task;
        }
    }

    new_node->next_task = cur->next_task;
    cur->next_task = new_node;
}


dd_task* remove_from_list(volatile dd_task_list **list, uint32_t task_id) {
    dd_task_list *cur = *list;
    dd_task_list *prev = NULL;
    dd_task *removed_task = NULL;

    while (cur != NULL && cur->task->task_id != task_id) {
        prev = cur;
        cur = cur->next_task;
    }

    if (cur != NULL) {
        if (prev != NULL) {
            prev->next_task = cur->next_task;
        } else {
            *list = cur->next_task;
        }

        removed_task = cur->task;
        free(cur);
    } else {
        printf("Task %d not found in list.\n", (int)task_id);
    }

    return removed_task;
}


void print_list(const volatile dd_task_list *list) {
    const dd_task_list *cur = list;
    while (cur !=  NULL) {
        printf("Rel: %d, Comp: %d, Ded: %d\n", (int)cur->task->release, (int)cur->task->completion, (int)cur->task->deadline);
        cur = cur->next_task;
    }
}
