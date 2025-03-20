#ifndef TASK_SCHEDULER_H
#define TASK_SCHEDULER_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdint.h>

/* Define the two possible task types */
typedef enum {
    PERIODIC,
    APERIODIC
} task_type;

/* Structure for a Deadline-Driven Task */
typedef struct {
    TaskHandle_t t_handle;
    task_type type;
    uint32_t task_id;
    uint32_t release_time;
    uint32_t absolute_deadline;
    uint32_t completion_time;
} dd_task;

/* Linked-list node for storing DD-Tasks */
typedef struct dd_task_list {
    dd_task task;
    struct dd_task_list *next_task;
} dd_task_list;

#ifdef __cplusplus
extern "C" {
#endif

// DDS interface functions used by other modules:
void release_dd_task(TaskHandle_t t_handle, task_type type, uint32_t task_id, uint32_t absolute_deadline);
void complete_dd_task(uint32_t task_id);
dd_task_list* get_active_dd_task_list(void);
dd_task_list* get_complete_dd_task_list(void);
dd_task_list* get_overdue_dd_task_list(void);

// Initialization function for the scheduler module.
void init_dd_scheduler(void);

#ifdef __cplusplus
}
#endif

#endif /* TASK_SCHEDULER_H */
