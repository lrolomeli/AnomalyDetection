#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "tcp_client.h"

void vClientTask(void *pvParameters)
{
    Socket_t xSocket;
    struct freertos_sockaddr xServerAddress;
    char cTxBuffer[BUFFER_SIZE] = "Hello from FreeRTOS TCP client";
    char cRxBuffer[BUFFER_SIZE];
    BaseType_t xBytesSent, xBytesReceived;

    // Create the socket
    xSocket = FreeRTOS_socket(FREERTOS_AF_INET,
                              FREERTOS_SOCK_STREAM,
                              FREERTOS_IPPROTO_TCP);

    // Check if the socket was created successfully
    if (xSocket != FREERTOS_INVALID_SOCKET)
    {
        // Set up the server address to connect to
        xServerAddress.sin_addr = FreeRTOS_inet_addr(SERVER_IP_ADDRESS);
        xServerAddress.sin_port = FreeRTOS_htons(SERVER_PORT);

        // Connect to the server
        if (FreeRTOS_connect(xSocket, &xServerAddress, sizeof(xServerAddress)) == 0)
        {
            // Send data to the server
            xBytesSent = FreeRTOS_send(xSocket, cTxBuffer, strlen(cTxBuffer), 0);

            if (xBytesSent > 0)
            {
                // Receive the echoed data back from the server
                xBytesReceived = FreeRTOS_recv(xSocket, cRxBuffer, BUFFER_SIZE, 0);

                if (xBytesReceived > 0)
                {
                    // Process the received data (in this case, just printing it)
                    printf("Received from server: %.*s\n", (int)xBytesReceived, cRxBuffer);
                }
            }
        }

        // Close the socket once done
        FreeRTOS_closesocket(xSocket);
    }

    vTaskDelete(NULL);
}
