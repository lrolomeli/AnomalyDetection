#include "my_lib/pingpong.h"

static uint8_t mic_buff_A[PPBSIZE] = {0};
static uint8_t mic_buff_B[PPBSIZE] = {0};

// almacenando 2 segundos del acelerometro
static uint8_t accel_buff_A[3750] = {0};
static uint8_t accel_buff_B[3750] = {0};

// 
static uint8_t * mic_full = NULL;
static uint8_t * mic_empty = NULL;

static uint8_t * accel_full = NULL;
static uint8_t * accel_empty = NULL;

uint8_t * get_bufferA(buff_func_t func)
{
	if(func == MIC){
		return mic_buff_A;
	}
	else if(func == ACCEL){
		return accel_buff_A;
	}
	else{
		return NULL;
	}
}

uint8_t * get_bufferB(buff_func_t func)
{
	
	if(func == MIC){
		return mic_buff_B;
	}
	else if(func == ACCEL){
		return accel_buff_B;
	}
	else{
		return NULL;
	}
}

void buffer_full(uint8_t * buffer, buff_func_t func)
{
	
	if(func == MIC){
		mic_full = buffer;
	}
	else if(func == ACCEL){
		accel_full = buffer;
	}
	else{

	}
}

void buffer_empty(uint8_t * buffer, buff_func_t func)
{
	
	if(func == MIC){
		mic_empty = buffer;
	}
	else if(func == ACCEL){
		accel_empty = buffer;
	}
	else{

	}
}

uint8_t * get_empty_buffer(buff_func_t func)
{
	if(func == MIC){
		return mic_empty;
	}
	else if(func == ACCEL){
		return accel_empty;
	}
	else{
		return NULL;
	}
}

uint8_t * get_full_buffer(buff_func_t func)
{
	if(func == MIC){
		return mic_full;
	}
	else if(func == ACCEL){
		return accel_full;
	}
	else{
		return NULL;
	}
}

uint8_t * swap_buffer(uint8_t * current, buff_func_t func)
{
	if(func == MIC){
		if(current != mic_buff_B) return mic_buff_B;
		else return mic_buff_A;
	}
	else if(func == ACCEL){
		if(current != mic_buff_B) return mic_buff_B;
		else return mic_buff_A;
	}
	else{
		return NULL;
	}
}