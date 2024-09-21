#ifndef APP_TASKS_H
#define APP_TASKS_H

#include "free_rtos.h"

#define ALIVE_FEATURE
#define DEBUG_IMPORTANT

//#define DEBUG_FEATURE

#define ADC_FEATURE
#define SDCARD_FEATURE
//#define NETWORK_FEATURE

//#define ACCEL_FEATURE
//#define DEBUG_MPU6050
//#define GYRO_FEATURE

void createFreeRTOSenv();

#endif /*APP_TASKS_H*/