#include "app.h"

int main()
{
    stdio_init_all();

    #ifdef ADC_FEATURE
    init_adc();
    #endif

    #ifdef ACCEL_FEATURE
    mpu6050_init();
    #endif

    if (cyw43_arch_init()) {
        printf("failed to initialise\n");
		return -1;
    }

    cyw43_arch_enable_sta_mode();
	
    #ifdef NETWORK_FEATURE
	tcp_start();
    #endif
    
    // Initialize SD card
    #ifdef SDCARD_FEATURE
    time_init();
    if (!sd_init_driver()) while(true);
    #endif
    
    createFreeRTOSenv();

    #ifdef DEBUG_IMPORTANT
    printf("Tasks Created Successfully!\r\n");
    #endif
    
    #ifdef ADC_FEATURE
    set_timadc_cb();
    #endif

    #ifdef DEBUG_IMPORTANT
    printf("OS Scheduler Starting!\r\n");
    #endif
    
    vTaskStartScheduler();

    return 0;
}