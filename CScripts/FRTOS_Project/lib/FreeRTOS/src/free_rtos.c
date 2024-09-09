#include "free_rtos.h"

TaskHandle_t xProcessTaskHandle = NULL;
TaskHandle_t xSendTaskHandle = NULL;

TaskHandle_t getProcessTaskHandler(){
    return xProcessTaskHandle;
}

TaskHandle_t getSendTaskHandler(){
    return xSendTaskHandle;
}

TaskHandle_t * getpProcessTaskHandler(){
    return &xProcessTaskHandle;
}

TaskHandle_t * getpSendTaskHandler(){
    return &xSendTaskHandle;
}