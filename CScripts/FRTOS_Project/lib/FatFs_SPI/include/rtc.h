#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "hardware/rtc.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/util/datetime.h"

#define FORMATTED_BUFSIZE 25

void time_init();
void get_formatted_datetime(char *buffer);

#ifdef __cplusplus
}
#endif
