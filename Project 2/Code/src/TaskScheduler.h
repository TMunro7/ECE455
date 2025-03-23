#ifndef TASK_SCHEDULER_H
#define TASK_SCHEDULER_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdint.h>

<<<<<<< HEAD
void DDScheduler(void *pvParameters);
void create_dd_task(TaskHandle_t task_h, task_type type, uint32_t task_id, uint32_t release, uint32_t deadline);
void delete_dd_task(uint32_t task_id);
=======
//NEED THIS STUFF FROM LINE 10-32

/* Define the two possible task types */
typedef enum {
    PERIODIC,
    APERIODIC
} task_type;
>>>>>>> dfe923a03919ae491dbffad9e3b2fa7c54d540c8

struct dd_task_list *get_active_dd_task_list(void);
struct dd_task_list *get_complete_dd_task_list(void);
struct dd_task_list *get_overdue_dd_task_list(void);

void push_to_list(volatile dd_task_list **list, dd_task *new_task);
void push_to_list_edf(volatile dd_task_list **list, dd_task *new_task);
dd_task *remove_from_list(volatile dd_task_list **list, uint32_t task_id);
void print_list(const volatile dd_task_list *list);

#endif /* TASK_SCHEDULER_H */
