#pragma once
#include <stdint.h>
typedef struct KeyboardStates
{
	uint32_t W;
	uint32_t A;
	uint32_t S;
	uint32_t D;
	uint32_t Q;
	uint32_t E;
	uint32_t F;
	uint32_t LEFT;
	uint32_t RIGHT;
	uint32_t SPACE;
} KeyboardStates;