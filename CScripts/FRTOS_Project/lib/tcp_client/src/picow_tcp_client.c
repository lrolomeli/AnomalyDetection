/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "picow_tcp_client.h"
#include <string.h> // Required for memset

#define TEST_TCP_SERVER_IP "192.168.100.248"
#define TCP_PORT 5001
#define DEBUG_printf printf

#define POLL_TIME_S 5

static TCP_CLIENT_T* tcp_client_init(void);
static err_t tcp_client_close(void *arg);
static err_t tcp_result(void *arg, int status);
static err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
static err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err);
static void tcp_client_err(void *arg, err_t err);
bool tcp_client_write(void *arg);
void reconnect(TCP_CLIENT_T * state);

#if 0
static void dump_bytes(const uint8_t *bptr, uint32_t len) {
    unsigned int i = 0;

    printf("dump_bytes %d", len);
    for (i = 0; i < len;) {
        if ((i & 0x0f) == 0) {
            printf("\n");
        } else if ((i & 0x07) == 0) {
            printf(" ");
        }
        printf("%02x ", bptr[i++]);
    }
    printf("\n");
}
#define DUMP_BYTES dump_bytes
#else
#define DUMP_BYTES(A,B)
#endif

// Perform initialisation
static TCP_CLIENT_T* tcp_client_init(void)
{
    TCP_CLIENT_T *state = calloc(1, sizeof(TCP_CLIENT_T));
    if (!state) {
        DEBUG_printf("failed to allocate state\n");
        return NULL;
    }
    ip4addr_aton(TEST_TCP_SERVER_IP, &state->remote_addr);
    return state;
}

static err_t tcp_client_close(void *arg)
{
    TCP_CLIENT_T *state = (TCP_CLIENT_T*)arg;
    err_t err = ERR_OK;
    if (state->tcp_pcb != NULL) {
        tcp_arg(state->tcp_pcb, NULL);
        tcp_poll(state->tcp_pcb, NULL, 0);
        tcp_sent(state->tcp_pcb, NULL);
        tcp_recv(state->tcp_pcb, NULL);
        tcp_err(state->tcp_pcb, NULL);
        err = tcp_close(state->tcp_pcb);
        if (err != ERR_OK) {
            DEBUG_printf("close failed %d, calling abort\n", err);
            tcp_abort(state->tcp_pcb);
            err = ERR_ABRT;
        }
        state->tcp_pcb = NULL;
    }
    return err;
}

// Called with results of operation
static err_t tcp_result(void *arg, int status)
{
    TCP_CLIENT_T *state = (TCP_CLIENT_T*)arg;
    if (status == 0) {
        DEBUG_printf("test success\n");
    } else {
        DEBUG_printf("test failed %d\n", status);
    }
    state->complete = true;
    return tcp_client_close(arg);
}

static err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
    TCP_CLIENT_T *state = (TCP_CLIENT_T*)arg;
    DEBUG_printf("tcp_client_sent %u\n", len);
    state->sent_len += len;

    if (state->sent_len >= BUF_SIZE) {

        state->run_count++;
        // We should receive a new buffer from the server
        state->buffer_len = 0;
        state->sent_len = 0;
        DEBUG_printf("Waiting for buffer from server\n");
    }

    return ERR_OK;
}

static err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
	TCP_CLIENT_T *state = (TCP_CLIENT_T*)arg;
    
	if (err == ERR_OK) {
		// Connection established, update the state
		state->connected = true;
		DEBUG_printf("We are ready to begin sending...\n");
		return ERR_OK;
    }
	else
	{
		// Handle connection error
		state->connected = false;
		printf("Could not connect to server: ERROR-%d\n", err);
        return tcp_result(arg, err);
	}
 
}

static void tcp_client_err(void *arg, err_t err)
{
    if (err != ERR_ABRT) {
        DEBUG_printf("tcp_client_err %d\n", err);
        tcp_result(arg, err);
    }
}

bool tcp_client_write(void *arg)
{
    err_t err = ERR_OK;
    TCP_CLIENT_T *state = (TCP_CLIENT_T*)arg;
    // DEBUG_printf("Connecting to %s port %u\n", ip4addr_ntoa(&state->remote_addr), TCP_PORT);
    state->tcp_pcb = tcp_new_ip_type(IP_GET_TYPE(&state->remote_addr));
	
    if (!state->tcp_pcb) {
        DEBUG_printf("failed to create pcb\n");
        return false;
    }

    tcp_arg(state->tcp_pcb, state);
    tcp_sent(state->tcp_pcb, tcp_client_sent);
    tcp_err(state->tcp_pcb, tcp_client_err);
	
    // cyw43_arch_lwip_begin/end should be used around calls into lwIP to ensure correct locking.
    // You can omit them if you are in a callback from lwIP. Note that when using pico_cyw_arch_poll
    // these calls are a no-op and can be omitted, but it is a good practice to use them in
    // case you switch the cyw43_arch type later.
    cyw43_arch_lwip_begin();
    err = tcp_connect(state->tcp_pcb, &state->remote_addr, TCP_PORT, tcp_client_connected);
    cyw43_arch_lwip_end();

    return ERR_OK == err;

}

void fillBufferWith(void *arg, uint8_t symbol)
{
    TCP_CLIENT_T *state = (TCP_CLIENT_T*)arg;
    memset(state->buffer, symbol, sizeof(state->buffer));
	state->buffer_len = BUF_SIZE;
}

void reconnect(TCP_CLIENT_T * state)
{
    if (!tcp_client_write(state)) {
        tcp_result(state, -1);
    }
}

TCP_CLIENT_T * tcp_socket()
{
	TCP_CLIENT_T *state = tcp_client_init();
    if (!state) {
        return state;
    }
    if (!tcp_client_write(state)) {
        tcp_result(state, -1);
        return state;
    }
	
	return state;
	
}

void send_data(TCP_CLIENT_T * state)
{
	err_t err = ERR_OK;
	if (true == state->connected) 
    {
		// Your connection is established, now send data
        cyw43_arch_lwip_begin();
		err = tcp_write(state->tcp_pcb, state->buffer, state->buffer_len, TCP_WRITE_FLAG_COPY);
        cyw43_arch_lwip_end();
		if (err != ERR_OK) {
			// Handle tcp_write error
            printf("WRITE ERROR: %d\n", err);
			tcp_result(state, err);
		}
        cyw43_arch_lwip_begin();
		err = tcp_output(state->tcp_pcb);
        cyw43_arch_lwip_end();
		if (err != ERR_OK) {
			// Handle tcp_output error
            printf("OUTPUT ERROR: %d\n", err);
			tcp_result(state, err);
		}

    }
    else
    {
        reconnect(state);
    }
}

void tcp_start(void) 
{
    if (cyw43_arch_init()) {
        DEBUG_printf("failed to initialise\n");
		return;
    }
    cyw43_arch_enable_sta_mode();

    printf("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("%s\n",WIFI_SSID);
		printf("%s\n",WIFI_PASSWORD);
		printf("failed to connect.\n");
        return;
    } else {
        printf("Connected.\n");
    }
	
    // cyw43_arch_deinit();

}