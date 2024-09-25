#include "my_lib/pingpong.h"



static uint8_t bufferA[PPBSIZE] = {0};
static uint8_t bufferB[PPBSIZE] = {0};
static uint8_t * full = NULL;
static uint8_t * empty = NULL;

uint8_t * get_bufferA()
{
	return bufferA;
}

uint8_t * get_bufferB()
{
	return bufferB;
}

void buffer_full(uint8_t * buffer)
{
	full = buffer;
}

void buffer_empty(uint8_t * buffer)
{
	empty = buffer;
}

uint8_t * get_empty_buffer()
{
	return empty;
}

uint8_t * get_full_buffer()
{
	return full;
}

uint8_t * swap_buffer(uint8_t * current)
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