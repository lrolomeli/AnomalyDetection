#ifndef __PICOW_TCP_CLIENT_H__
#define __PICOW_TCP_CLIENT_H__

#include <string.h>
#include <time.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#define BUF_SIZE 2048

typedef struct TCP_CLIENT_T_ {
    struct tcp_pcb *tcp_pcb;
    ip_addr_t remote_addr;
    uint8_t buffer[BUF_SIZE];
    int buffer_len;
    int sent_len;
    bool complete;
    int run_count;
    bool connected;
} TCP_CLIENT_T;

void tcp_start();
TCP_CLIENT_T * tcp_socket();
void send_data(TCP_CLIENT_T * state);
void fillBufferWith(void *arg, uint8_t symbol);

#endif /* __PICOW_TCP_CLIENT_H__ */
