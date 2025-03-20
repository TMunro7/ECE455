#include "TaskScheduler.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>

/* DDS task priority and message queue parameters */
#define DDS_TASK_PRIORITY      (configMAX_PRIORITIES - 1)
#define DDS_QUEUE_LENGTH       10

/* Define priorities for user-defined F-Tasks. The scheduler assigns:
   - F_TASK_HIGH_PRIORITY: to the task with the earliest deadline
   - F_TASK_LOW_PRIORITY: to all other tasks
*/
#define F_TASK_HIGH_PRIORITY   (configMAX_PRIORITIES - 2)
#define F_TASK_LOW_PRIORITY    (configMAX_PRIORITIES - 3)

/* Message types for DDS communication */
typedef enum {
    MSG_RELEASE_TASK,
    MSG_COMPLETE_TASK,
    MSG_GET_ACTIVE_LIST,
    MSG_GET_COMPLETED_LIST,
    MSG_GET_OVERDUE_LIST
} dd_msg_type;

/* Structure for messages sent to the DDS task */
typedef struct {
    dd_msg_type type;
    union {
        struct {
            TaskHandle_t t_handle;
            task_type type;
            uint32_t task_id;
            uint32_t absolute_deadline;
        } release;
        struct {
            uint32_t task_id;
        } complete;
    } data;
    QueueHandle_t responseQueue;  // Used for GET requests to return the list pointer
} dd_msg;

/* Global variables for the DDS */
static QueueHandle_t ddQueue = NULL;
static dd_task_list *activeList = NULL;
static dd_task_list *completedList = NULL;
static dd_task_list *overdueList = NULL;

/* Inserts a new DD-Task into the active list in sorted order (by absolute_deadline) */
static void insert_task_sorted(dd_task new_task)
{
    dd_task_list *newNode = pvPortMalloc(sizeof(dd_task_list));
    if(newNode == NULL) {
        /* Handle memory allocation failure if needed */
        return;
    }
    newNode->task = new_task;
    newNode->next_task = NULL;
    
    if(activeList == NULL || new_task.absolute_deadline < activeList->task.absolute_deadline) {
        newNode->next_task = activeList;
        activeList = newNode;
    } else {
        dd_task_list *curr = activeList;
        while(curr->next_task != NULL && 
              curr->next_task->task.absolute_deadline <= new_task.absolute_deadline) {
            curr = curr->next_task;
        }
        newNode->next_task = curr->next_task;
        curr->next_task = newNode;
    }
}

/* Removes and returns a node from activeList that matches task_id */
static dd_task_list* remove_task_from_active(uint32_t task_id)
{
    dd_task_list *prev = NULL;
    dd_task_list *curr = activeList;
    while(curr != NULL) {
        if(curr->task.task_id == task_id) {
            if(prev == NULL) {
                activeList = curr->next_task;
            } else {
                prev->next_task = curr->next_task;
            }
            curr->next_task = NULL; // detach node
            return curr;
        }
        prev = curr;
        curr = curr->next_task;
    }
    return NULL;
}

/* Adjusts the priorities of tasks in the active list:
   - The task with the earliest deadline gets high priority.
   - All others get low priority.
*/
static void adjust_priorities(void)
{
    dd_task_list *node = activeList;
    if(node != NULL) {
        if(node->task.t_handle != NULL) {
            vTaskPrioritySet(node->task.t_handle, F_TASK_HIGH_PRIORITY);
        }
        node = node->next_task;
        while(node != NULL) {
            if(node->task.t_handle != NULL) {
                vTaskPrioritySet(node->task.t_handle, F_TASK_LOW_PRIORITY);
            }
            node = node->next_task;
        }
    }
}

/* Checks the active list for any tasks that have missed their deadlines and moves them to the overdue list */
static void check_overdue_tasks(void)
{
    TickType_t current_tick = xTaskGetTickCount();
    dd_task_list *prev = NULL;
    dd_task_list *curr = activeList;
    while(curr != NULL) {
        if(curr->task.absolute_deadline < current_tick) {
            dd_task_list *overdueNode = curr;
            if(prev == NULL) {
                activeList = curr->next_task;
                curr = activeList;
            } else {
                prev->next_task = curr->next_task;
                curr = prev->next_task;
            }
            overdueNode->next_task = overdueList;
            overdueList = overdueNode;
        } else {
            prev = curr;
            curr = curr->next_task;
        }
    }
}

/* The main DDS task. It waits on the ddQueue for messages and processes them accordingly. */
static void vTaskScheduler(void *pvParameters)
{
    dd_msg msg;
    (void) pvParameters;
    
    for(;;) {
        if(xQueueReceive(ddQueue, &msg, portMAX_DELAY) == pdTRUE) {
            switch(msg.type) {
                case MSG_RELEASE_TASK:
                {
                    dd_task new_task;
                    new_task.t_handle = msg.data.release.t_handle;
                    new_task.type = msg.data.release.type;
                    new_task.task_id = msg.data.release.task_id;
                    new_task.release_time = xTaskGetTickCount();
                    new_task.absolute_deadline = msg.data.release.absolute_deadline;
                    new_task.completion_time = 0;
                    
                    insert_task_sorted(new_task);
                    adjust_priorities();
                    break;
                }
                case MSG_COMPLETE_TASK:
                {
                    dd_task_list *completedNode = remove_task_from_active(msg.data.complete.task_id);
                    if(completedNode != NULL) {
                        completedNode->task.completion_time = xTaskGetTickCount();
                        completedNode->next_task = completedList;
                        completedList = completedNode;
                    }
                    adjust_priorities();
                    break;
                }
                case MSG_GET_ACTIVE_LIST:
                {
                    if(msg.responseQueue != NULL) {
                        xQueueSend(msg.responseQueue, &activeList, portMAX_DELAY);
                    }
                    break;
                }
                case MSG_GET_COMPLETED_LIST:
                {
                    if(msg.responseQueue != NULL) {
                        xQueueSend(msg.responseQueue, &completedList, portMAX_DELAY);
                    }
                    break;
                }
                case MSG_GET_OVERDUE_LIST:
                {
                    if(msg.responseQueue != NULL) {
                        xQueueSend(msg.responseQueue, &overdueList, portMAX_DELAY);
                    }
                    break;
                }
                default:
                    break;
            }
        }
        /* After processing a message, check for overdue tasks and update priorities */
        check_overdue_tasks();
        adjust_priorities();
    }
}

/* Public API function to create a new DD-Task.
   This function packages the task information in a message and sends it to the DDS queue.
*/
void release_dd_task(TaskHandle_t t_handle, task_type type, uint32_t task_id, uint32_t absolute_deadline)
{
    dd_msg msg;
    msg.type = MSG_RELEASE_TASK;
    msg.data.release.t_handle = t_handle;
    msg.data.release.type = type;
    msg.data.release.task_id = task_id;
    msg.data.release.absolute_deadline = absolute_deadline;
    msg.responseQueue = NULL;
    
    xQueueSend(ddQueue, &msg, portMAX_DELAY);
}

/* Public API function indicating that a DD-Task has completed its execution.
   It sends a message to the DDS to update the DD-Task lists.
*/
void complete_dd_task(uint32_t task_id)
{
    dd_msg msg;
    msg.type = MSG_COMPLETE_TASK;
    msg.data.complete.task_id = task_id;
    msg.responseQueue = NULL;
    
    xQueueSend(ddQueue, &msg, portMAX_DELAY);
}

/* Returns the pointer to the active DD-Task list.
   This function uses a temporary response queue to obtain the list pointer from the DDS.
*/
dd_task_list* get_active_dd_task_list(void)
{
    dd_msg msg;
    dd_task_list *list = NULL;
    msg.type = MSG_GET_ACTIVE_LIST;
    msg.responseQueue = xQueueCreate(1, sizeof(dd_task_list*));
    xQueueSend(ddQueue, &msg, portMAX_DELAY);
    xQueueReceive(msg.responseQueue, &list, portMAX_DELAY);
    vQueueDelete(msg.responseQueue);
    return list;
}

/* Returns the pointer to the completed DD-Task list */
dd_task_list* get_complete_dd_task_list(void)
{
    dd_msg msg;
    dd_task_list *list = NULL;
    msg.type = MSG_GET_COMPLETED_LIST;
    msg.responseQueue = xQueueCreate(1, sizeof(dd_task_list*));
    xQueueSend(ddQueue, &msg, portMAX_DELAY);
    xQueueReceive(msg.responseQueue, &list, portMAX_DELAY);
    vQueueDelete(msg.responseQueue);
    return list;
}

/* Returns the pointer to the overdue DD-Task list */
dd_task_list* get_overdue_dd_task_list(void)
{
    dd_msg msg;
    dd_task_list *list = NULL;
    msg.type = MSG_GET_OVERDUE_LIST;
    msg.responseQueue = xQueueCreate(1, sizeof(dd_task_list*));
    xQueueSend(ddQueue, &msg, portMAX_DELAY);
    xQueueReceive(msg.responseQueue, &list, portMAX_DELAY);
    vQueueDelete(msg.responseQueue);
    return list;
}

/* Initializes the DDS module by creating the message queue and starting the scheduler task */
void init_dd_scheduler(void)
{
    ddQueue = xQueueCreate(DDS_QUEUE_LENGTH, sizeof(dd_msg));
    if(ddQueue == NULL) {
        printf("Failed to create DDS queue\n");
        return;
    }
    xTaskCreate(vTaskScheduler, "DDS", configMINIMAL_STACK_SIZE, NULL, DDS_TASK_PRIORITY, NULL);
}
