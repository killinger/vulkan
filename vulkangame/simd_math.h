#pragma once
#include <stdint.h>
#include <xmmintrin.h>


// NOTE: +Y is down in Vulkan
// https://software.intel.com/sites/landingpage/IntrinsicsGuide/

typedef struct Vector3
{
	float x;
	float y;
	float z;
} Vector3;

typedef struct Vector4
{
	float x;
	float y;
	float z;
	float w;
} Vector4;

// sqrt(x^2 + y^2 + z^2)
float getVectorLength(Vector3 vector)
{
	// TODO: profile
	__m128 A = _mm_set_ps(0, vector.z, vector.y, vector.x);
	__m128 mul = _mm_mul_ps(A, A);
	__declspec(align(16)) float powValues[4];
	_mm_store_ps(powValues, mul);
	float add = powValues[0] + powValues[1] + powValues[2];
	A = _mm_load_ss(&add);
	__m128 root = _mm_sqrt_ss(A);
	float result = 0;
	_mm_store_ss(&result, root);
	return result;
}

// (1/vectorLength) * vector
Vector3 convertVectorToUnitLength(Vector3 vector)
{
	// TODO: profile
	__m128 A = _mm_set1_ps(1/getVectorLength(vector));
	__m128 B = _mm_set_ps(0, vector.z, vector.y, vector.x);
	__m128 unitLengths = _mm_mul_ps(A, B);
	__declspec(align(16)) float lengths[4];
	_mm_store_ps(lengths, unitLengths);
	Vector3 result = {
		lengths[0],
		lengths[1],
		lengths[2]
	};
	return result;
}