#include "app.h"
#include "mpu6050.h"
#include "picow_tcp_client.h"
#include "my_lib/pingpong.h"
#include "adc_lib.h"
#include "f_util.h"
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
static uint32_t err_cnt = 0;

void led_task(void * pvParameters)
{   
    // tiempo en milisegundos
	TickType_t xLastWakeTime;
	const  TickType_t xFrequency = 1000 / portTICK_PERIOD_MS;
	BaseType_t xWasDelayed;
    uint uiVal = 0;

	// Initialize the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, uiVal);
        uiVal = (uiVal) ? 0 : 1;
        xWasDelayed = xTaskDelayUntil(&xLastWakeTime, xFrequency);
		(void) xWasDelayed;
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

        #ifdef DEBUG_MPU6050
        printf("Accel: X=%d Y=%d Z=%d\n", accel_x, accel_y, accel_z);
        #endif

        #ifdef GYRO_FEATURE
		// Read gyroscope values
        mpu6050_read_gyro(I2C_PORT, &gyro_x, &gyro_y, &gyro_z);
		// Print the values

        #ifdef DEBUG_MPU6050
		printf("Gyro: X=%d Y=%d Z=%d\n", gyro_x, gyro_y, gyro_z);
        #endif

        #endif

		xWasDelayed = xTaskDelayUntil(&xLastWakeTime, xFrequency);
		(void) xWasDelayed;
	}
	
}

// Task to process the ADC readings after buffer is full
void vProcessingTask(void *pvParameters) 
{    
    FATFS fs;
    FRESULT fr;
    FIL fil;
    int ret;
    const char filename[] = "test001.csv";
    uint8_t times235 = 0;
    uint32_t row = 1;
	
    #ifdef DEBUG_IMPORTANT
    printf("Starting processing task\n");
    #endif

	// Mount drive
    fr = f_mount(&fs, "0:", 1);
    if(FR_OK != fr) while(true);

    fr = f_open(&fil, filename, FA_WRITE | FA_OPEN_APPEND | FA_CREATE_ALWAYS);
    if(FR_OK != fr) while(true);

    for(;;) 
    {
        // Wait until notified by the ADC reading task
        #ifdef ADC_FEATURE
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        processing_buffer = get_full_buffer();
        #else
        processing_buffer = get_bufferA();
        #endif
        if(235 > times235)
        {
            for(uint16_t i=0;i<SIZE;i++)
            {
                // Write something to file
                ret = f_printf(&fil, "%d,%d\n", row, processing_buffer[i]);
                if(ret < 0)
                {
                    // Just retry to write the same value
                    // By leaving the row the same
                    // and substracting 1 to i
                    err_cnt++;
                    i--;
                }
                else{
                    row++;
                }
            }
            times235++;
        }
        else
        {
            #ifdef ADC_FEATURE
            stop_adc_sampling();
            #endif
            // Close file
            fr = f_close(&fil);

            if(FR_OK != fr) while(true);

            #ifdef DEBUG_IMPORTANT
            printf("Done with processing task error count: %d\n", err_cnt);
            #endif

            // Unmount drive
            f_unmount("0:");

            times235 = 0;

            #ifdef NETWORK_FEATURE
            xTaskNotifyGive(getSendTaskHandler());
            #else
            vTaskDelete(NULL);
            #endif
        }
    }
}

// Task to process the ADC readings after buffer is full
void tcp_send_task(void *pvParameters) 
{
    err_t err = ERR_OK;
    TCP_CLIENT_T * socket = tcp_socket();
    bool buffsel = true;
    for(;;) 
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
    /* Check if the task was created successfully */
    #ifdef SDCARD_FEATURE
    xTaskCreate(vProcessingTask, "SD_Task", 8192, NULL, 1, getpProcessTaskHandler());
    #endif

    #ifdef NETWORK_FEATURE
    xTaskCreate(tcp_send_task, "TCP_Task", 4096, NULL, 1, getpSendTaskHandler());
    #endif

    #ifdef ACCEL_FEATURE
    xTaskCreate(accel_task, "ACCEL_Task", 256, NULL, 1, NULL);
    #endif

    #ifdef ALIVE_FEATURE
    xTaskCreate(led_task, "ALIVE_Task", 256, NULL, 1, NULL);
    #endif
}