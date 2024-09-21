#ifndef PING_PONG_H
#define PING_PONG_H

#include <stdio.h>

#define SIZE 4096

uint16_t * get_bufferA();
uint16_t * get_bufferB();
uint16_t * swap_buffer(uint16_t * current);
void buffer_empty(uint16_t * buffer);
void buffer_full(uint16_t * buffer);
uint16_t * get_empty_buffer();
uint16_t * get_full_buffer();

#endif /*PING_PONG_H*/