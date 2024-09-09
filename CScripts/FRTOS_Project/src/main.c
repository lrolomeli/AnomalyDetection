#include "app.h"
#include "adc_lib.h"
#include "mpu6050.h"
#include "picow_tcp_client.h"

int main()
{
    stdio_init_all();
    init_adc();
	mpu6050_init();
	tcp_start();
    createFreeRTOSenv();
    start_adc_sampling();
    vTaskStartScheduler();

    return 0;
}