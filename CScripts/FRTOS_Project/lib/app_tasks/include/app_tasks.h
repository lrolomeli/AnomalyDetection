#ifndef APP_TASKS_H
#define APP_TASKS_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

void createFreeRTOSenv();
TaskHandle_t getProcessTaskHandler();
TaskHandle_t getSendTaskHandler();

#endif /*APP_TASKS_H*/