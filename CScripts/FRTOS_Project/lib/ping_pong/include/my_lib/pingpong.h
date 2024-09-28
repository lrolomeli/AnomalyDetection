#ifndef PING_PONG_H
#define PING_PONG_H

#include <stdio.h>

#define SIZE 4096
#define PPBSIZE SIZE*2

typedef enum{
    MIC = 0,
    ACCEL = 1,
    HALL = 2
}buff_func_t;

uint8_t * get_bufferA(buff_func_t func);
uint8_t * get_bufferB(buff_func_t func);
void buffer_full(uint8_t * buffer, buff_func_t func);
void buffer_empty(uint8_t * buffer, buff_func_t func);
uint8_t * get_empty_buffer(buff_func_t func);
uint8_t * get_full_buffer(buff_func_t func);
uint8_t * swap_buffer(uint8_t * current, buff_func_t func);

#endif /*PING_PONG_H*/