#pragma once
#include "vkrenderer.h"
#include "simd_math.h"
#include "collision_detection.h"

typedef enum ObjectType
{
	OBJECT_BACKGROUND,
	OBJECT_BOX
} ObjectType;

typedef enum PlayerStates
{
	STATE_GROUNDED,
	STATE_JUMPING,
	STATE_FALLING
} PlayerStates;

typedef enum Position
{
	POSITION_CENTERED
} Position;

typedef struct Hitbox
{
	ObjectRenderResources renderResources;
	Rect rectangle;
	uint32_t colorIndex;
	uint32_t collisionColorIndex;
} Hitbox;

typedef struct Object
{
	ObjectRenderResources renderResources;
	ObjectType type;
	Hitbox hitbox;
	Vector2 position;
	Vector2 scale;
	float rotation;
} Object;

typedef struct ObjectGroup
{
	Hitbox hitbox;
	Vector2 position;
	uint32_t objectCount;
	Object *objects;
} ObjectGroup;

typedef struct Player
{
	ObjectRenderResources renderResources;
	Hitbox hitbox;
	PlayerStates state;
	Vector2 position;
	Vector2 velocity;
	Vector2 scale;
	Vector2 maxVelocity;
	float startAcceleration;
	float constantSpeedBreakpoint;
	float airAcceleration;
	float jumpVelocity;
	float rotation;
} Player;