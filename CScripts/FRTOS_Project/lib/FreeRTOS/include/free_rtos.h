#ifndef MY_FREE_RTOS_H
#define MY_FREE_RTOS_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

TaskHandle_t getProcessTaskHandler();
TaskHandle_t getSendTaskHandler();
TaskHandle_t * getpProcessTaskHandler();
TaskHandle_t * getpSendTaskHandler();

#endif /*MY_FREE_RTOS_H*/