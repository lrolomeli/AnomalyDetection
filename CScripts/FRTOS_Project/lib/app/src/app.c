#include "app.h"

#define ONEMINBLOCK 235

void usb_task(void * pvParameters);
void led_task(void * pvParameters);
void send_task(void * pvParameters);
void accel_task(void * pvParameters);
void vProcessingTask(void * pvParameters);

// Buffer for storing ADC readings
char filename[FORMATTED_BUFSIZE];
static uint32_t err_cnt = 0;
static FATFS fs;
static FIL fil;
static FRESULT fr;

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
    int ret;
    uint8_t times235 = 0;
    uint32_t row = 1;
    bool new_capture = true;
    uint16_t * processing_buffer = NULL;

    // Mount drive
    if(FR_OK != f_mount(&fs, "0:", 1)) while(true);

    for(;;) 
    {
        if(new_capture)
        {
            new_capture = false;
            get_formatted_datetime(filename);    
	
            #ifdef DEBUG_IMPORTANT
            printf("%s\r\n", filename);
            printf("Starting processing task\n");
            #endif
            
            fr = f_open(&fil, filename, FA_WRITE | FA_OPEN_APPEND | FA_CREATE_ALWAYS);
            if(FR_OK != fr) while(true);
        }
        // Wait until notified by the ADC reading task
        #ifdef ADC_FEATURE
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        processing_buffer = get_full_buffer();
        #else
        processing_buffer = get_bufferA();
        #endif
        if(ONEMINBLOCK > times235)
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
            printf("%d packets left:\r", ONEMINBLOCK - times235);
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

            times235 = 0;
            new_capture = true;

            #ifdef NETWORK_FEATURE
            xTaskNotifyGive(getSendTaskHandler());
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            #else
            // Unmount drive
            f_unmount("0:");
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

    char buf[16];
    char *token;
    uint16_t buffill = 0;
    uint16_t num = 0;
    uint16_t dbgcnt = 0;
    uint8_t big = 0;
    uint8_t little = 0;

    for(;;) 
    {
        // Wait until notified by the ADC reading task
        #ifdef SDCARD_FEATURE
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        #endif

        fr = f_open(&fil, filename, FA_READ);
        if(FR_OK != fr) while(true);

        while (f_gets(buf, sizeof(buf), &fil)) 
        {
            // Split the string using ',' as the delimiter
            token = strtok(buf, ",");  // First part (ignore this)
            token = strtok(NULL, ","); // Second part (this is the number "2048")
            // Convert the extracted string to an integer
            num = (uint16_t)atoi(token);

            little = num & 0xFF;
            big = (num >> 8) & 0xFF;

            socket->buffer[(buffill*2)] = little;
            socket->buffer[(buffill*2)+1] = big;
            #ifdef TESTING
            printf("%d | ", num);
            #endif
            buffill++;

            if(buffill == DATA_SIZE)
            {
                //printf("First buffer number: %d\n",socket->buffer[0]);
                //printf("Buffer Ready to be transmitted: %d\n",buffill);
                while(!retry_wifi_conn()){
                    vTaskDelay(1000);
                }
                printf("Sending Buffer #%d...\r", ++dbgcnt);
                buffill = 0;
                socket->buffer_len = BUF_SIZE;
                send_data(socket);
                while(!waitAck()){
                    vTaskDelay(5);
                }
                resetAck();
            }

        }

    }
}

void createFreeRTOSenv()
{
    /* Check if the task was created successfully */
    #ifdef SDCARD_FEATURE
    xTaskCreate(vProcessingTask, "SD_Task", 8192, NULL, 1, getpProcessTaskHandler());
    #endif

    #ifdef NETWORK_FEATURE
    xTaskCreate(tcp_send_task, "TCP_Task", 8192, NULL, 1, getpSendTaskHandler());
    #endif

    #ifdef ACCEL_FEATURE
    xTaskCreate(accel_task, "ACCEL_Task", 256, NULL, 1, NULL);
    #endif

    #ifdef ALIVE_FEATURE
    xTaskCreate(led_task, "ALIVE_Task", 256, NULL, 1, NULL);
    #endif
}