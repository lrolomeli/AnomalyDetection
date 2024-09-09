#ifndef ADC_LIB_H
#define ADC_LIB_H

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/timer.h"

void init_adc();
void stop_adc_sampling();
void start_adc_sampling();

#endif /*ADC_LIB_H*/