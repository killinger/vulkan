#pragma once
#include <stdint.h>
#include <xmmintrin.h>
#define _USE_MATH_DEFINES   
#include <math.h>  

// NOTE: +Y is down in Vulkan
// https://software.intel.com/sites/landingpage/IntrinsicsGuide/
//
//typedef struct Vector3
//{
//	float x;
//	float y;
//	float z;
//} Vector3;
//
//typedef struct Vector4
//{
//	float x;
//	float y;
//	float z;
//	float w;
//} Vector4;
//
//// sqrt(x^2 + y^2 + z^2)
//float getVectorLength(Vector3 vector)
//{
//	// TODO: profile
//	__m128 A = _mm_set_ps(0, vector.z, vector.y, vector.x);
//	__m128 mul = _mm_mul_ps(A, A);
//	__declspec(align(16)) float powValues[4];
//	_mm_store_ps(powValues, mul);
//	float add = powValues[0] + powValues[1] + powValues[2];
//	A = _mm_load_ss(&add);
//	__m128 root = _mm_sqrt_ss(A);
//	float result = 0;
//	_mm_store_ss(&result, root);
//	return result;
//}
//
//// (1/vectorLength) * vector
//Vector3 convertVectorToUnitLength(Vector3 vector)
//{
//	// TODO: profile
//	__m128 A = _mm_set1_ps(1/getVectorLength(vector));
//	__m128 B = _mm_set_ps(0, vector.z, vector.y, vector.x);
//	__m128 unitLengths = _mm_mul_ps(A, B);
//	__declspec(align(16)) float lengths[4];
//	_mm_store_ps(lengths, unitLengths);
//	Vector3 result = {
//		lengths[0],
//		lengths[1],
//		lengths[2]
//	};
//	return result;
//}

typedef struct Vector2
{
	float x;
	float y;
} Vector2;

typedef struct Vector4
{
	float e[4];
} Vector4;

typedef struct Matrix4x4
{
	float e[16];
} Matrix4x4;

Matrix4x4 createOrthographicProjectionMatrix(
	float leftPlane,
	float rightPlane,
	float topPlane,
	float bottomPlane,
	float nearPlane,
	float farPlane);

Matrix4x4 createTranslationMatrix(
	float x,
	float y,
	float z);

Matrix4x4 createScalingMatrix2D(float xScale, float yScale);

Matrix4x4 createRotationMatrix2D(float angle);

// TODO: simd implementation?
Matrix4x4 multiply_m2(Matrix4x4 m1, Matrix4x4 m2);

Matrix4x4 multiply_m3(Matrix4x4 m1, Matrix4x4 m2, Matrix4x4 m3);

Matrix4x4 createMatrixSetDiagonals(float x, float y, float z, float h);
Matrix4x4 lookAt(Vector4 e, Vector4 g, Vector4 t);
Vector4 createVector4(float x, float y, float z, float h);
Vector4 crossProduct(Vector4 a, Vector4 b);
float magnitude(Vector4 v);
void clamp(float min, float max, float *value);
float degreesToRadians(float degrees);