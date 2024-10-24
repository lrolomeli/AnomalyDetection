#include "my_lib/pingpong.h"

// 

static uint8_t mA[PPBSIZE]={0};
static uint8_t mB[PPBSIZE]={0};
static uint8_t aA[3750]={0};
static uint8_t aB[3750]={0};

static buffer_t bmA = {&mA[0],Empty};
static buffer_t bmB = {&mB[0],Empty};
static buffer_t baA = {&aA[0],Empty};
static buffer_t baB = {&aB[0],Empty};

static pingpong_t ppmic = {&bmA[0], &bmB[0]};
// almacenando 2 segundos del acelerometro
static pingpong_t ppaccel = {&baA[0], &baB[0]};

pingpong_t * getPP(uint8_t size){
	if(size == PPBSIZE)
		return ppmic;
	else
		return ppaccel;
}

void swap_buffer(pingpong_t * pp)
{
	buffer_t * temp = pp->str;
	pp->str = pp->rd;
	pp->rd = temp;
}