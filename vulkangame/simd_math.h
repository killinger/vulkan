#pragma once
#include <stdint.h>
#include <xmmintrin.h>


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
	float farPlane)
{
	Matrix4x4 matrix = { 0 };

	matrix.e[0] = 2.0f / (rightPlane - leftPlane);
	matrix.e[1] = 0.0f;
	matrix.e[2] = 0.0f;
	matrix.e[3] = 0.0f;

	matrix.e[4] = 0.0f;
	matrix.e[5] = 2.0f / (bottomPlane - topPlane);
	matrix.e[6] = 0.0f;
	matrix.e[7] = 0.0f;

	matrix.e[8] = 0.0f;
	matrix.e[9] = 0.0f;
	matrix.e[10] = 1.0f / (nearPlane - farPlane);
	matrix.e[11] = 0.0f;

	matrix.e[12] = -(rightPlane + leftPlane) / (rightPlane - leftPlane);
	matrix.e[13] = -(bottomPlane + topPlane) / (bottomPlane - topPlane);
	matrix.e[14] = nearPlane / (nearPlane - farPlane);
	matrix.e[15] = 1.0f;

	return matrix;
}

Matrix4x4 createTranslationMatrix(
	float x,
	float y,
	float z)
{
	Matrix4x4 matrix = { 0 };

	matrix.e[0] = 1.0f;
	matrix.e[1] = 0.0f;
	matrix.e[2] = 0.0f;
	matrix.e[3] = 0.0f;

	matrix.e[4] = 0.0f;
	matrix.e[5] = 1.0f;
	matrix.e[6] = 0.0f;
	matrix.e[7] = 0.0f;

	matrix.e[8] = 0.0f;
	matrix.e[9] = 0.0f;
	matrix.e[10] = 1.0f;
	matrix.e[11] = 0.0f;

	matrix.e[12] = x;
	matrix.e[13] = y;
	matrix.e[14] = z;
	matrix.e[15] = 1.0f;

	return matrix;
}

// TODO: simd implementation?
Matrix4x4 multiply(Matrix4x4 m1, Matrix4x4 m2)
{
	Matrix4x4 result = { 0 };

	// column 1
	result.e[0] = (m1.e[0] * m2.e[0]) + (m1.e[4] * m2.e[1]) + (m1.e[8] * m2.e[2]) + (m1.e[12] * m2.e[3]);
	result.e[1] = (m1.e[1] * m2.e[0]) + (m1.e[5] * m2.e[1]) + (m1.e[9] * m2.e[2]) + (m1.e[13] * m2.e[3]);
	result.e[2] = (m1.e[2] * m2.e[0]) + (m1.e[6] * m2.e[1]) + (m1.e[10] * m2.e[2]) + (m1.e[14] * m2.e[3]);
	result.e[3] = (m1.e[3] * m2.e[0]) + (m1.e[7] * m2.e[1]) + (m1.e[11] * m2.e[2]) + (m1.e[15] * m2.e[3]);

	// column 2
	result.e[4] = (m1.e[0] * m2.e[4]) + (m1.e[4] * m2.e[5]) + (m1.e[8] * m2.e[6]) + (m1.e[12] * m2.e[7]);
	result.e[5] = (m1.e[1] * m2.e[4]) + (m1.e[5] * m2.e[5]) + (m1.e[9] * m2.e[6]) + (m1.e[13] * m2.e[7]);
	result.e[6] = (m1.e[2] * m2.e[4]) + (m1.e[6] * m2.e[5]) + (m1.e[10] * m2.e[6]) + (m1.e[14] * m2.e[7]);
	result.e[7] = (m1.e[3] * m2.e[4]) + (m1.e[7] * m2.e[5]) + (m1.e[11] * m2.e[6]) + (m1.e[15] * m2.e[7]);

	// column 3
	result.e[8] = (m1.e[0] * m2.e[8]) + (m1.e[4] * m2.e[9]) + (m1.e[8] * m2.e[10]) + (m1.e[12] * m2.e[11]);
	result.e[9] = (m1.e[1] * m2.e[8]) + (m1.e[5] * m2.e[9]) + (m1.e[9] * m2.e[10]) + (m1.e[13] * m2.e[11]);
	result.e[10] = (m1.e[2] * m2.e[8]) + (m1.e[6] * m2.e[9]) + (m1.e[10] * m2.e[10]) + (m1.e[14] * m2.e[11]);
	result.e[11] = (m1.e[3] * m2.e[8]) + (m1.e[7] * m2.e[5]) + (m1.e[11] * m2.e[10]) + (m1.e[15] * m2.e[11]);

	// column 4
	result.e[12] = (m1.e[0] * m2.e[12]) + (m1.e[4] * m2.e[13]) + (m1.e[8] * m2.e[14]) + (m1.e[12] * m2.e[15]);
	result.e[13] = (m1.e[1] * m2.e[12]) + (m1.e[5] * m2.e[13]) + (m1.e[9] * m2.e[14]) + (m1.e[13] * m2.e[15]);
	result.e[14] = (m1.e[2] * m2.e[12]) + (m1.e[6] * m2.e[13]) + (m1.e[10] * m2.e[14]) + (m1.e[14] * m2.e[15]);
	result.e[15] = (m1.e[3] * m2.e[12]) + (m1.e[7] * m2.e[13]) + (m1.e[11] * m2.e[14]) + (m1.e[15] * m2.e[15]);

	return result;
}

Matrix4x4 createMatrixSetDiagonals(float x, float z, float y, float h)
{
	Matrix4x4 matrix = { 0 };

	matrix.e[0] = x;
	matrix.e[1] = 0.0f;
	matrix.e[2] = 0.0f;
	matrix.e[3] = 0.0f;

	matrix.e[4] = 0.0f;
	matrix.e[5] = y;
	matrix.e[6] = 0.0f;
	matrix.e[7] = 0.0f;

	matrix.e[8] = 0.0f;
	matrix.e[9] = 0.0f;
	matrix.e[10] = z;
	matrix.e[11] = 0.0f;

	matrix.e[12] = 0.0f;
	matrix.e[13] = 0.0f;
	matrix.e[14] = 0.0f;
	matrix.e[15] = h;

	return matrix;
}