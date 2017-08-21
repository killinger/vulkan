#pragma once
#include "simd_math.h"

typedef struct Camera2D
{
	Matrix4x4 projection;
	Matrix4x4 vp;
	float rotation;
	float xPosition;
	float yPosition;
	float xScaleFactor;
	float yScaleFactor;
	float speed;
} Camera2D;

Camera2D c_createCamera2D(float width, float height)
{
	Camera2D camera = { 0 };
	camera.projection = createOrthographicProjectionMatrix(
		-width / 2, width / 2, 
		-height / 2, height / 2, 
		0.1f, 10.0f);
	camera.rotation = 0.0f;
	camera.xPosition = 0.0f;
	camera.yPosition = 0.0f;
	camera.speed = 2.0f;
	return camera;
}