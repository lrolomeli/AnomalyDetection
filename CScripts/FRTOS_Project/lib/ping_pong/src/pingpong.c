#include "my_lib/pingpong.h"

#define SIZE 2048

static uint16_t bufferA[SIZE] = {0};
static uint16_t bufferB[SIZE] = {0};
static uint16_t * full = NULL;
static uint16_t * empty = NULL;

uint16_t * get_bufferA()
{
	return bufferA;
}

uint16_t * get_bufferB()
{
	return bufferB;
}

void buffer_full(uint16_t * buffer)
{
	full = buffer;
}

void buffer_empty(uint16_t * buffer)
{
	empty = buffer;
}

uint16_t * get_empty_buffer()
{
	return empty;
}

uint16_t * get_full_buffer()
{
	return full;
}

uint16_t * swap_buffer(uint16_t * current)
{
	if(current != bufferB)
	{
		return bufferB;
	}
	else
	{
		return bufferA;
	}
}