#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "mpu6050.h"
#include "picow_tcp_client.h"
#include "hardware/adc.h"
#include "hardware/timer.h"

// Constants
#define SAMPLE_RATE_US 62.5
#define NUM_SAMPLES 1024
#define MINUTE1COUNT 960000

void usb_task(void * pvParameters);
void led_task(void * pvParameters);
void send_task(void * pvParameters);
void accel_task(void * pvParameters);
void adcTask(void *pvParameters);
void stop_adc_sampling();
void start_adc_sampling();
volatile bool timer_active = false;
struct repeating_timer timer;

TaskHandle_t xProcessTaskHandle = NULL;
TaskHandle_t xSendTaskHandle = NULL;
static QueueHandle_t xQueue = NULL;
// Buffer for storing ADC readings
static uint8_t bufferA[BUF_SIZE] = {0};
static uint8_t bufferB[BUF_SIZE] = {0};
volatile uint16_t sample_count = 0;
volatile bool fill_times = false;
static uint8_t * adc_buffer = bufferA;
static uint8_t * processing_buffer = NULL;
static volatile uint32_t timer_one_min = 0;

// Timer interrupt callback function
bool repeating_timer_callback(struct repeating_timer *t) {
    // Read ADC value
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint16_t adc_value = adc_read();
    bool switch_task = false;
    
    // Store ADC value in buffer
    // Split the 16-bit value into two 8-bit values
    adc_buffer[(sample_count*2) % BUF_SIZE] = (adc_value >> 8) & 0xFF; // Extract the high byte
    adc_buffer[((sample_count*2) + 1) % BUF_SIZE] = adc_value & 0xFF;  // Extract the low byte

    sample_count++;
    timer_one_min++;

    if(MINUTE1COUNT == timer_one_min){
        timer_one_min = 0;
        // Notify the processing task
        vTaskNotifyGiveFromISR(xSendTaskHandle, &xHigherPriorityTaskWoken);
        switch_task = true;
    }

    // Check if buffer is full
    if (sample_count >= NUM_SAMPLES && MINUTE1COUNT == timer_one_min) {
        if(fill_times){
            adc_buffer = bufferA;
            fill_times = false;
        }
        else{
            adc_buffer = bufferB;
            fill_times = true;
        }
        sample_count = 0;  // Reset sample counter

        // Notify the processing task
        vTaskNotifyGiveFromISR(xProcessTaskHandle, &xHigherPriorityTaskWoken);
        switch_task = true;
    }
    if(switch_task)
    {
        switch_task = false;
        // Context switch if necessary
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

    // Return true to keep the timer repeating
    return true;
}

// Function to stop ADC sampling
void stop_adc_sampling() 
{
    // Stop the repeating timer
    if (timer_active) {
        cancel_repeating_timer(&timer);
        timer_active = false;
    }
    
    // Turn off ADC
    adc_run(false);
    printf("ADC sampling stopped.\n");
}

// Function to start ADC sampling
void start_adc_sampling() 
{
    // Turn on ADC
    adc_run(true);
    
    // Start the repeating timer
    if (!timer_active) {
        add_repeating_timer_us(-SAMPLE_RATE_US, repeating_timer_callback, NULL, &timer);
        timer_active = true;
    }
    
    printf("ADC sampling started.\n");
}

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
void vProcessTask(void *pvParameters) 
{
    err_t err = ERR_OK;
    bool buffsel = true;
    while (1) 
    {
        // Wait until notified by the ADC reading task
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if(buffsel)
        {
            buffsel = false;
            // Process the data in adc_buffer
            processing_buffer = bufferA;
        }
        else
        {
            buffsel = true;
            // Process the data in adc_buffer
            processing_buffer = bufferB;
        }
        sd_store(processing_buffer);
        vTaskNotifyGive(xSendTaskHandle);
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
        if(buffsel)
        {
            buffsel = false;
            // Process the data in adc_buffer
            socket->buffer = bufferA;
        }
        else
        {
            buffsel = true;
            // Process the data in adc_buffer
            socket->buffer = bufferB;
        }
        socket->buffer_len = BUF_SIZE;
        send_data(socket);
    }
}

int main()
{
    stdio_init_all(); // Initialize I/O
    xQueue = xQueueCreate(1, sizeof(uint));


    adc_init(); // Initialize the ADC hardware
    adc_gpio_init(26); // GPIO 26 corresponds to ADC channel 0
    adc_select_input(0); // Select ADC channel 0
    
    start_adc_sampling();
	mpu6050_init();
	tcp_start();
	
	xTaskCreate(accel_task, "ACCEL_Task", 256, NULL, 1, NULL);
	xTaskCreate(vProcessTask, "ADC_Task", 4096, NULL, 1, &xProcessTaskHandle);
    xTaskCreate(tcp_send_task, "ADC_Task", 4096, NULL, 1, &xSendTaskHandle);
    
    xTaskCreate(usb_task, "USB_Task", 256, NULL, 1, NULL);
    xTaskCreate(led_task, "LED_Task", 256, NULL, 1, NULL);
    vTaskStartScheduler();

    return 0;
}