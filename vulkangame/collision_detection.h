#pragma once
#include <stdint.h>
#include "simd_math.h"

typedef struct Rect
{
	Vector2 bottomLeft;
	Vector2 bottomRight;
	Vector2 topLeft;
	Vector2 topRight;
} Rect;

uint32_t rectanglesIntersect(Rect r1, Vector2 position1, Rect r2, Vector2 position2, Vector2 *adjustmentVector);