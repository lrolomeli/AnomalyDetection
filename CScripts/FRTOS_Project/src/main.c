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

void adc_init();
void usb_task(void * pvParameters);
void led_task(void * pvParameters);
void send_task(void * pvParameters);
void accel_task(void * pvParameters);
void adcTask(void *pvParameters);

TaskHandle_t xProcessTaskHandle = NULL;
static QueueHandle_t xQueue = NULL;
// Buffer for storing ADC readings
volatile uint16_t adc_buffer[NUM_SAMPLES];
volatile uint16_t sample_count = 0;

// Timer interrupt callback function
bool repeating_timer_callback(struct repeating_timer *t) {
    // Read ADC value
    uint16_t adc_value = adc_read();

    // Store ADC value in buffer
    adc_buffer[sample_count] = adc_value;
    sample_count++;

    // Check if buffer is full
    if (sample_count >= NUM_SAMPLES) {
        sample_count = 0;  // Reset sample counter
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        
        // Notify the processing task
        vTaskNotifyGiveFromISR(xProcessTaskHandle, &xHigherPriorityTaskWoken);

        // Context switch if necessary
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

    // Return true to keep the timer repeating
    return true;
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

void send_task(void * pvParameters)
{
	err_t err = ERR_OK;
    TCP_CLIENT_T * socket = tcp_socket();
	
	for(;;)
	{
        fillBufferWith(socket, 'M');
        printf("ERROR: %d\n", send_data(socket));
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

        printf("Accel: X=%d Y=%d Z=%d\n", accel_x, accel_y, accel_z);
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
    while (1) 
    {
        // Wait until notified by the ADC reading task
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // Process the data in adc_buffer
        for (int i = 0; i < NUM_SAMPLES; i++) 
        {
            // Example processing (simple print)
            printf("ADC Value[%d]: %d\n", i, adc_buffer[i]);
        }
    }
}

int main()
{
    //xQueue = xQueueCreate(1, sizeof(uint));
    // Setup the repeating timer to fire every 62.5 microseconds
    //struct repeating_timer timer;
    //add_repeating_timer_us(-SAMPLE_RATE_US, repeating_timer_callback, NULL, &timer);


    stdio_init_all(); // Initialize I/O

    //adc_init(); // Initialize the ADC hardware
    //adc_gpio_init(26); // GPIO 26 corresponds to ADC channel 0
    //adc_select_input(0); // Select ADC channel 0

	//mpu6050_init();
	tcp_start();
	
	xTaskCreate(send_task, "TCP_Task", 4096, NULL, 1, NULL);
	//xTaskCreate(accel_task, "ACCEL_Task", 256, NULL, 1, NULL);
	//xTaskCreate(vProcessTask, "ADC_Task", 256, NULL, 1, &xProcessTaskHandle);
    //xTaskCreate(usb_task, "USB_Task", 256, NULL, 1, NULL);
    //xTaskCreate(led_task, "LED_Task", 256, NULL, 1, NULL);
    vTaskStartScheduler();

    return 0;
}