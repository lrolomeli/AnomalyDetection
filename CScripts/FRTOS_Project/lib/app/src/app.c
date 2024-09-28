#include "app.h"

// 62.5 Hz = 62.5 samples per second
// each sample 2 bytes = 125 bytes per second
// 3 axis per sample = 125 * 3 = 375 bytes per second
// 10 seconds = 3750 buffer size 
// 6 times = 1 min samples
#define ACCEL_ONEMINBLOCK 6
#define TCPACCEL2KBTX (ACCEL_ONEMINBLOCK << 2)
#define ONEMINBLOCK (235U)
#define TCP2KBTX (ONEMINBLOCK << 2)
#define ENABLE_TCP_TX

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
    uint16_t accel[3] = {0};
    uint16_t sample_count = 0;
    uint8_t * mpu_buffer = NULL;
	//int16_t accel_x, accel_y, accel_z;
	//int16_t gyro_x, gyro_y, gyro_z;

	// Initialize the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();

	for(;;)
	{
		// Read accelerometer values
        mpu6050_read_accel(&accel[0], &accel[1], &accel[2]);

        // Store ADC value in buffer
        for(uint8_t i=0; i<3; i++){
            mpu_buffer[sample_count] = accel[i] & 0xFF;
            mpu_buffer[sample_count + 1] = (accel[i] >> 8) & 0xFF;
            sample_count += 2;
        }

        // Check if buffer is full
        if (sample_count >= 3750) {
            buffer_full(mpu_buffer, ACCEL);
            mpu_buffer = swap_buffer(mpu_buffer, ACCEL);
            sample_count = 0;  // Reset sample counter
            xTaskNotifyGive(getAccelStorTaskHandler());
        }

		xWasDelayed = xTaskDelayUntil(&xLastWakeTime, xFrequency);
		(void) xWasDelayed;
	}
	
}

void vSaveAccelValues(void *pvParameters) {
    int ret;
    uint8_t writed_blocks = 0;
    uint32_t row = 1;
    bool new_capture = true;
    uint8_t * processing_buffer = NULL;
    UINT rec_num = 0;

        if(new_capture)
        {
            new_capture = false;
            get_formatted_datetime(filename, 'a');    
	
            #ifdef DEBUG_IMPORTANT
            printf("%s\r\n", filename);
            printf("Starting processing task\n");
            #endif
            
            fr = f_open(&fil, filename, FA_WRITE | FA_OPEN_APPEND | FA_CREATE_ALWAYS);
            if(FR_OK != fr) while(true);
        }
        // Wait until notified by the ADC reading task
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        processing_buffer = get_full_buffer(ACCEL);

        if( writed_blocks < ACCEL_ONEMINBLOCK )
        {
            ERR_OK != f_write(&fil, processing_buffer, 3750, &rec_num);
            writed_blocks++;
            printf("%d packets left:\r", (ACCEL_ONEMINBLOCK - writed_blocks));
        }
        else
        {
            // Close file
            fr = f_close(&fil);
        }
}

// Task to process the ADC readings after buffer is full
void vSaveMicSamples(void *pvParameters) 
{
    int ret;
    uint8_t writed_blocks = 0;
    uint32_t row = 1;
    bool new_capture = true;
    uint8_t * processing_buffer = NULL;
    UINT rec_num = 0;

    // Mount drive
    if(FR_OK != f_mount(&fs, "0:", 1)) while(true);

    for(;;) 
    {
        if(new_capture)
        {
            new_capture = false;
            get_formatted_datetime(filename, 'm');    
	
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
        processing_buffer = get_full_buffer(MIC);
        #else
        processing_buffer = get_bufferA();
        #endif
        if( writed_blocks < ONEMINBLOCK )
        {
            ERR_OK != f_write(&fil, processing_buffer, PPBSIZE, &rec_num);
            writed_blocks++;
            printf("%d packets left:\r", (ONEMINBLOCK - writed_blocks));
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

            writed_blocks = 0;
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
    UINT bytesRead = 0;
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

        for(UINT read_block=0; read_block < TCP2KBTX; read_block++)
        {
            // Read 2048 bytes
            err = f_read(&fil, socket->buffer, BUF_SIZE, &bytesRead);
            if (err != FR_OK || bytesRead < BUF_SIZE) {
            // Handle error
            }
            #ifndef ENABLE_TCP_TX
            printf("%x|%x - %x|%x\n", socket->buffer[0],socket->buffer[1],socket->buffer[2046],socket->buffer[2047]);
            #else
            while(!retry_wifi_conn())
            {
                vTaskDelay(1000);
            }
            printf("Sending Buffer #%d...\r", read_block);
            socket->buffer_len = BUF_SIZE;
            send_data(socket);
            while(!waitAck())
            {
                vTaskDelay(2);
            }
            resetAck();
            #endif
        }
    }
}

void createFreeRTOSenv()
{
    /* Check if the task was created successfully */
    #ifdef SDCARD_FEATURE
    xTaskCreate(vSaveMicSamples, "SD_Task", 8192, NULL, 3, getpProcessTaskHandler());
    #endif

    #ifdef NETWORK_FEATURE
    xTaskCreate(tcp_send_task, "TCP_Task", 8192, NULL, 1, getpSendTaskHandler());
    #endif

    #ifdef ACCEL_FEATURE
    xTaskCreate(accel_task, "ACCEL_Task", 256, NULL, 4, NULL);
    xTaskCreate(vSaveAccelValues, "SD_Task", 8192, NULL, 2, getpAccelStorTaskHandler());
    #endif

    #ifdef ALIVE_FEATURE
    xTaskCreate(led_task, "ALIVE_Task", 256, NULL, 1, NULL);
    #endif
}