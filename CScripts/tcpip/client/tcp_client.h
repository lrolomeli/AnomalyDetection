#ifndef TCP_TASKS_H
#define TCP_TASKS_H

#include "FreeRTOS.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

// Definiciones de puerto y tamaño del buffer
#define SERVER_PORT     5001
#define BUFFER_SIZE     128

// Dirección IP del servidor
#define SERVER_IP_ADDRESS "192.168.1.100"  // Cambiar según sea necesario

// Declaraciones de funciones para tareas
void vClientTask(void *pvParameters);

#endif /* TCP_TASKS_H */
