#include "app.h"
#include "mpu6050.h"
#include "picow_tcp_client.h"
#include "my_lib/pingpong.h"
#include "sd_card.h"
#include "ff.h"

void usb_task(void * pvParameters);
void led_task(void * pvParameters);
void send_task(void * pvParameters);
void accel_task(void * pvParameters);
void vProcessingTask(void * pvParameters);

static QueueHandle_t xQueue = NULL;
// Buffer for storing ADC readings

static uint16_t * processing_buffer = NULL;

void usb_task(void * pvParameters)
{
    uint uiValRx = 0;
    while(true)
    {
        xQueueReceive(xQueue, &uiValRx, portMAX_DELAY);

        if(uiValRx == 1){
            printf("LED is ON! \n");
        }
        else{
            printf("LED is OFF! \n");
        }
    }
}

void led_task(void * pvParameters)
{   
    uint uiValTx = 0;
    while (true) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        uiValTx = 1;
        xQueueSend(xQueue, &uiValTx, 0U);
        vTaskDelay(1000);

        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        uiValTx = 0;
        xQueueSend(xQueue, &uiValTx, 0U);
        vTaskDelay(1000);
    }
}

void accel_task(void * pvParameters)
{
	// tiempo en milisegundos
	TickType_t xLastWakeTime;
	const  TickType_t xFrequency = 16 / portTICK_PERIOD_MS;
	BaseType_t xWasDelayed;
	int16_t accel_x, accel_y, accel_z;
	//int16_t gyro_x, gyro_y, gyro_z;

	// Initialize the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();

	for(;;)
	{
		// Read accelerometer values
        mpu6050_read_accel(&accel_x, &accel_y, &accel_z);

        // printf("Accel: X=%d Y=%d Z=%d\n", accel_x, accel_y, accel_z);
		// Read gyroscope values
        // mpu6050_read_gyro(I2C_PORT, &gyro_x, &gyro_y, &gyro_z);
		// Print the values
		// printf("Gyro: X=%d Y=%d Z=%d\n", gyro_x, gyro_y, gyro_z);
		xWasDelayed = xTaskDelayUntil(&xLastWakeTime, xFrequency);
		(void) xWasDelayed;
	}
	
}

// Task to process the ADC readings after buffer is full
void vProcessingTask(void *pvParameters) 
{
    FRESULT fr = FR_OK;
    FATFS fs;
    FIL fil;
    int ret;
    char buf[100];
    char filename[] = "test.csv";
    // Mount drive
    fr = f_mount(&fs, "0:", 1);
    if(FR_OK != fr) printf("Error to mount disk/n");
    fr = f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS);
    if(FR_OK != fr) printf("Error to open file/n");
    if (f_printf(&fil, "1, 12394\n") < 0) printf("f_printf failed\n");
    fr = f_close(&fil);
    if (FR_OK != fr) printf("f_close error: (%d)\n", fr);
    f_unmount("0:");

    while (1) 
    {
        #if 0
        // Wait until notified by the ADC reading task
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        processing_buffer = get_full_buffer();
        //sd_store(processing_buffer);
        xTaskNotifyGive(getSendTaskHandler());
        #endif
        vTaskDelay(1000);
    }
}

// Task to process the ADC readings after buffer is full
void tcp_send_task(void *pvParameters) 
{
    err_t err = ERR_OK;
    TCP_CLIENT_T * socket = tcp_socket();
    bool buffsel = true;
    while (1) 
    {
        // Wait until notified by the ADC reading task
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        // socket->buffer = prepare4tcp(get_full_buffer());
        socket->buffer_len = BUF_SIZE;
        send_data(socket);
    }
}

void createFreeRTOSenv()
{
    xQueue = xQueueCreate(1, sizeof(uint));
	//xTaskCreate(accel_task, "ACCEL_Task", 256, NULL, 1, NULL);

    /* Check if the task was created successfully */
    if( pdPASS != xTaskCreate(vProcessingTask, "SD_Task", 4096, NULL, 1, getpProcessTaskHandler()) )
    {
        /* The task was created successfully and is ready to run. */
        printf("Task Failed!\n");
    }
    else{
        printf("The value of the handler must be something different to null %d, %d\n",getpProcessTaskHandler(), getProcessTaskHandler());
    }
    //xTaskCreate(tcp_send_task, "ADC_Task", 4096, NULL, 1, getpProcessTaskHandler());
    xTaskCreate(usb_task, "USB_Task", 256, NULL, 1, NULL);
    xTaskCreate(led_task, "LED_Task", 256, NULL, 1, NULL);
}