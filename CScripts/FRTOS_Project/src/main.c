#include "app.h"
#include "adc_lib.h"
#include "mpu6050.h"
#include "picow_tcp_client.h"
#include "sd_card.h"
#include "ff.h"

int main()
{
    stdio_init_all();
    //init_adc();
	//mpu6050_init();
	tcp_start();
    // Initialize SD card
    if (!sd_init_driver()) printf("ERROR: Could not initialize SD card\r\n");
    createFreeRTOSenv();
    //start_adc_sampling();
    vTaskStartScheduler();

    return 0;
}