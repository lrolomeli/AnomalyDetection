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

err_t tcp_client_close(void *arg);
err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
err_t tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
bool tcp_client_open(void *arg);

#endif /* __PICOW_TCP_CLIENT_H__ */
