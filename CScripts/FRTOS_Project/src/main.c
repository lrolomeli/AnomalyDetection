#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include "pico/cyw43_arch.h"
#include "picow_tcp_client.h"

static QueueHandle_t xQueue = NULL;

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
	TCP_CLIENT_T * socket = tcp_socket();
	err_t err = ERR_OK;
	
	for(;;)
	{
		err = send_data(socket);
		vTaskDelay(1000);
	}
	
}

int main()
{
    xQueue = xQueueCreate(1, sizeof(uint));
	
	tcp_start();
	
	xTaskCreate(send_task, "TCP_Task", 4096, NULL, 1, NULL);
    xTaskCreate(usb_task, "USB_Task", 256, NULL, 1, NULL);
    xTaskCreate(led_task, "LED_Task", 256, NULL, 1, NULL);
    vTaskStartScheduler();

    return 0;
}