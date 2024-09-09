#include "app.h"
#include "adc_lib.h"
#include "mpu6050.h"
#include "picow_tcp_client.h"
#include "sd_card.h"

int main()
{
    stdio_init_all();
    //init_adc();
	//mpu6050_init();
    //sd_init_driver();
	//tcp_start();
    createFreeRTOSenv();
    //start_adc_sampling();
    vTaskStartScheduler();

    return 0;
}