#include "my_lib/pingpong.h"
#include "free_rtos.h"
#include "adc_lib.h"

#define SAMPLE_RATE_US 62.5
#define SIZE 4096
#define PPBSIZE SIZE*2

static struct repeating_timer timer;
static uint16_t sample_count = 0;
static pingpong_t * adc_buffer = NULL;
static bool timadc_powst = false;

// Timer interrupt callback function
static bool repeating_timer_callback(struct repeating_timer *t) 
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	// Read ADC value
    uint16_t adc_value = adc_read();
    //printf("debug isr callback timer\n");
    
    // Store ADC value in buffer
    adc_buffer->str[sample_count] = adc_value & 0xFF;
    adc_buffer->str[sample_count + 1] = (adc_value >> 8) & 0xFF;

    sample_count += 2;

    // Check if buffer is full
    if (sample_count >= PPBSIZE) {
		adc_buffer->str.status = Full;
		swap_buffer(adc_buffer->str);
        sample_count = 0;  // Reset sample counter

        // Notify the processing task
        vTaskNotifyGiveFromISR(getProcessTaskHandler(), &xHigherPriorityTaskWoken);
        // Context switch if necessary
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

    // Return true to keep the timer repeating
    return true;
}

// Function to stop ADC sampling
void stop_adc_sampling() 
{
    if(timadc_powst == true)
    {
        // Stop the repeating timer
        cancel_repeating_timer(&timer);
        // Turn off ADC
        adc_run(false);
        printf("ADC sampling stopped.\n");
        timadc_powst = false;
    }
}

// Function to start ADC sampling
void start_adc_sampling() 
{
    if(timadc_powst == false)
    {
        // Turn on ADC
        adc_run(true);
        printf("debug3\n");
        // Start the repeating timer
        add_repeating_timer_us(-SAMPLE_RATE_US, repeating_timer_callback, NULL, &timer);    
        printf("ADC sampling started.\n");
        timadc_powst = true;
    }
}

void set_timadc_cb(){
    // Start the repeating timer
    add_repeating_timer_us(-SAMPLE_RATE_US, repeating_timer_callback, NULL, &timer);
    timadc_powst = true;
}

void init_adc()
{
	adc_init(); // Initialize the ADC hardware
    adc_gpio_init(26); // GPIO 26 corresponds to ADC channel 0
    adc_select_input(0); // Select ADC channel 0

	adc_buffer = getPP(PPBSIZE);
}