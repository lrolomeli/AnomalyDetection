#ifndef PING_PONG_H
#define PING_PONG_H

#include <stdio.h>

typedef enum{
	Empty = 0,
	Full = 1,
	Emptying = 2,
    Filling = 3
} buffState_t;

typedef struct {
	uint8_t * buf;
	buffState_t state;
}buffer_t;

typedef struct {
	buffer_t * str; // active buffer is being filled
	buffer_t * rd; // buffer that has been filled 

}pingpong_t;

pingpong_t * getPP(uint8_t size);
uint8_t * swap_buffer(uint8_t * current, buff_func_t func);

#endif /*PING_PONG_H*/