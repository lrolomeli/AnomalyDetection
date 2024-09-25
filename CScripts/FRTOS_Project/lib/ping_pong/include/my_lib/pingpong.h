#ifndef PING_PONG_H
#define PING_PONG_H

#include <stdio.h>

#define SIZE 4096
#define PPBSIZE SIZE*2

uint8_t * get_bufferA();
uint8_t * get_bufferB();
uint8_t * swap_buffer(uint8_t * current);
void buffer_empty(uint8_t * buffer);
void buffer_full(uint8_t * buffer);
uint8_t * get_empty_buffer();
uint8_t * get_full_buffer();

#endif /*PING_PONG_H*/