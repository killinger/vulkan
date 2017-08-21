#include <math.h>
#include "collision_detection.h"
#include "simd_math.h"

uint32_t rectanglesIntersect(Rect r1, Vector2 position1, Rect r2, Vector2 position2, Vector2 *adjustmentVector)
{
	Vector2 delta1 = { 0 };
	delta1.x = (r2.bottomLeft.x + position2.x) - (r1.topRight.x + position1.x);
	delta1.y = (r2.bottomLeft.y + position2.y) - (r1.topRight.y + position1.y);

	Vector2 delta2 = { 0 };
	delta2.x = (r1.bottomLeft.x + position1.x) - (r2.topRight.x + position2.x);
	delta2.y = (r1.bottomLeft.y + position1.y) - (r2.topRight.y + position2.y);

	if (delta1.x > 0.0f || delta1.y > 0.0f)
		return 0;
	if (delta2.x > 0.0f || delta2.y > 0.0f)
		return 0;

	if (adjustmentVector != NULL)
	{
		if (delta1.x > delta2.x)
			adjustmentVector->x = delta1.x - 0.01f;
		else
			adjustmentVector->x = -delta2.x + 0.01f;
		if (delta1.y > delta2.y)
			adjustmentVector->y = delta1.y  - 0.01f;
		else
			adjustmentVector->y = -delta2.y +0.01f;
	}
	return 1;
}