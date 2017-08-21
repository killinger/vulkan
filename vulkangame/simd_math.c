#include "simd_math.h"
#include <math.h>

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

Matrix4x4 createScalingMatrix2D(float xScale, float yScale)
{
	Matrix4x4 matrix = { 0 };

	matrix.e[0] = xScale;
	matrix.e[1] = 0.0f;
	matrix.e[2] = 0.0f;
	matrix.e[3] = 0.0f;

	matrix.e[4] = 0.0f;
	matrix.e[5] = yScale;
	matrix.e[6] = 0.0f;
	matrix.e[7] = 0.0f;

	matrix.e[8] = 0.0f;
	matrix.e[9] = 0.0f;
	matrix.e[10] = 1.0f;
	matrix.e[11] = 0.0f;

	matrix.e[12] = 0.0f;
	matrix.e[13] = 0.0f;
	matrix.e[14] = 0.0f;
	matrix.e[15] = 1.0f;

	return matrix;
}

Matrix4x4 createRotationMatrix2D(float angle)
{
	Matrix4x4 matrix = { 0 };

	matrix.e[0] = (float)cos((double)angle);
	matrix.e[1] = (float)sin((double)angle);
	matrix.e[2] = 0.0f;
	matrix.e[3] = 0.0f;

	matrix.e[4] = -(float)sin((double)angle);
	matrix.e[5] = (float)cos((double)angle);
	matrix.e[6] = 0.0f;
	matrix.e[7] = 0.0f;

	matrix.e[8] = 0.0f;
	matrix.e[9] = 0.0f;
	matrix.e[10] = 1.0f;
	matrix.e[11] = 0.0f;

	matrix.e[12] = 0.0f;
	matrix.e[13] = 0.0f;
	matrix.e[14] = 0.0f;
	matrix.e[15] = 1.0f;

	return matrix;
}

// TODO: simd implementation?
Matrix4x4 multiply_m2(Matrix4x4 m1, Matrix4x4 m2)
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

Matrix4x4 multiply_m3(Matrix4x4 m1, Matrix4x4 m2, Matrix4x4 m3)
{
	return multiply_m2(multiply_m2(m1, m2), m3);
}

Matrix4x4 createMatrixSetDiagonals(float x, float y, float z, float h)
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

Matrix4x4 lookAt(Vector4 e, Vector4 g, Vector4 t)
{
	float gMagnitude = magnitude(g);

	Vector4 w = { 0 };
	w.e[0] = -g.e[0] / gMagnitude;
	w.e[1] = -g.e[1] / gMagnitude;
	w.e[2] = -g.e[2] / gMagnitude;
	w.e[3] = 1.0f;

	Vector4 u = crossProduct(t, w);
	float uMagnitude = magnitude(u);

	u.e[0] = u.e[0] / uMagnitude;
	u.e[1] = u.e[2] / uMagnitude;
	u.e[1] = u.e[2] / uMagnitude;

	Vector4 v = crossProduct(w, u);

	Matrix4x4 m1 = { 0 };
	m1.e[0] = u.e[0];
	m1.e[1] = v.e[0];
	m1.e[2] = w.e[0];
	m1.e[3] = 0.0f;

	m1.e[4] = u.e[1];
	m1.e[5] = v.e[1];
	m1.e[6] = w.e[1];
	m1.e[7] = 0.0f;

	m1.e[8] = u.e[2];
	m1.e[9] = v.e[2];
	m1.e[10] = w.e[2];
	m1.e[11] = 0.0f;

	m1.e[12] = 0.0f;
	m1.e[13] = 0.0f;
	m1.e[14] = 0.0f;
	m1.e[15] = 1.0f;

	Matrix4x4 m2 = createMatrixSetDiagonals(1.0f, 1.0f, 1.0f, 1.0f);
	m2.e[12] = -e.e[0];
	m2.e[13] = -e.e[1];
	m2.e[14] = -e.e[2];

	return(multiply_m2(m1, m2));
}

Vector4 createVector4(float x, float y, float z, float h)
{
	Vector4 v = { 0 };
	v.e[0] = x;
	v.e[1] = y;
	v.e[2] = z;
	v.e[3] = h;

	return v;
}

Vector4 crossProduct(Vector4 a, Vector4 b)
{
	Vector4 cross = { 0 };
	cross.e[0] = (a.e[1] * b.e[2]) - (a.e[2] * b.e[1]);
	cross.e[1] = (a.e[2] * b.e[0]) - (a.e[0] * b.e[2]);
	cross.e[2] = (a.e[0] * b.e[1]) - (a.e[1] * b.e[0]);
	cross.e[3] = 1.0f;

	return cross;
}

float magnitude(Vector4 v)
{
	return (float)sqrt((double)(v.e[0] * v.e[0] + v.e[1] * v.e[1] + v.e[2] * v.e[2]));
}

void clamp(float min, float max, float *value)
{
	if (*value < min)
		*value = min;
	else if (*value > max)
		*value = max;
}

float degreesToRadians(float degrees)
{
	return degrees * ((float)M_PI / 180.0f);
}