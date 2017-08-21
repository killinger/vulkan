#define VK_USE_PLATFORM_WIN32_KHR

#include <Windows.h>
#include <assert.h>
#include <tchar.h>
#include "vulkan\vulkan.h"
#include "datatest.h"
#include "vkrenderer.h"
#include "camera.h"
#include "simd_math.h"
#include "keyboard_input.h"
#include "collision_detection.h"
#include "Entities.h"

#define MS_PER_SECOND 1000.0
#define MAX_FRAMERATE 60.0

void messagePump(MSG msg, HWND window);
void translateInput(uint32_t key, uint32_t state);
void handleInput();
Object createObject(
	ObjectType type,
	float width,
	float height,
	float x,
	float y,
	float rotation,
	float xScale,
	float yScale,
	uint32_t textureIndex);
Object createBox(float x, float y);
Hitbox createHitbox(float width, float height, uint32_t colorIndex, uint32_t collisionColorIndex, Position position);
Player createPlayer(float x, float y);
void updateBackground();
void updatePlayer(Player *player);
void updateCamera();
void render();

const double MAX_FRAME_TIME = MS_PER_SECOND / MAX_FRAMERATE;
const float UP_GRAVITY = 9.82f;
const float DOWN_GRAVITY = 9.82f;

Camera2D camera;
KeyboardStates keyboardStates;
Object background;
Object square;
Hitbox squareHitbox;
Object square2;
Hitbox squareHitbox2;
Object square3;
Hitbox squareHitbox3;
Player player;

uint32_t squareCount = 3;
Object *squares;
Hitbox *squareHitboxes;

uint32_t objectGroupCount;
ObjectGroup *objectGroups;

void dllTest()
{
	DataTest test = { 0 };
	HMODULE test_dll = LoadLibrary(L"test.dll");
	void(*func)(DataTest*) = (void*)GetProcAddress(test_dll, "testprint");
	func(&test);
	FreeLibrary(test_dll);
	int i = 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	HWND window = r_initVkRenderer(hInstance, nShowCmd);
	MSG msg = { 0 };

	camera.projection = createOrthographicProjectionMatrix(-1280.0f / 4, 1280.0f / 4, -720.0f / 4, 720.0f / 4, 0.1f, 10.0f);
	camera.rotation = 0.0f;
	camera.xPosition = 0.0f;
	camera.yPosition = 0.0f;
	camera.xScaleFactor = 2.0f;
	camera.yScaleFactor = 2.0f;
	camera.speed = 2.0f;

	Matrix4x4 lookAtMatrix = lookAt(
		createVector4(camera.xPosition, camera.yPosition, 0.0f, 1.0f),
		createVector4(0.0f, 0.0f, 1.0f, 1.0f),
		createVector4(0.0f, -1.0f, 0.0f, 1.0f)
	);

	camera.vp = multiply_m2(camera.projection, lookAtMatrix); // TODO: this aint right but it works

	// OBJECT GROUPS
	objectGroupCount = 1;
	objectGroups = (ObjectGroup*)malloc(objectGroupCount * sizeof(ObjectGroup));
	objectGroups[0].hitbox = createHitbox(128.0f, 128.0f, 2, 0, POSITION_CENTERED);
	objectGroups[0].position.x = 48.0f;
	objectGroups[0].position.y = 48.0f;
	objectGroups[0].objectCount = 3;
	objectGroups[0].objects = (Object*)malloc(objectGroups[0].objectCount * sizeof(Object));
	objectGroups[0].objects[0] = createBox(32.0f, 0.0f);
	objectGroups[0].objects[1] = createBox(64.0f, 0.0f);
	objectGroups[0].objects[2] = createBox(0.0f, 32.0f);

	//
	player = createPlayer(0.0f, 64.0f);

	background = createObject(OBJECT_BACKGROUND, 1280.0f, 720.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 2);
	background.renderResources.textureDisplacement[0] = 1.0f / 9.0f;
	background.renderResources.textureDisplacement[1] = 1.0f;
	background.renderResources.textureDisplacement[2] = 4.0f;
	background.renderResources.textureDisplacement[3] = 0.0f;

	while (msg.message != WM_QUIT)
	{
		messagePump(msg, window);

		LARGE_INTEGER startPerformanceCount;
		LARGE_INTEGER endPerformanceCount;
		LARGE_INTEGER performanceFrequency;
		QueryPerformanceCounter(&startPerformanceCount);
		QueryPerformanceFrequency(&performanceFrequency);
		double cpuFrequency = (double)performanceFrequency.QuadPart / 1000.0;

		r_beginFrame();

		handleInput();
		updateCamera();
		updateBackground();
		updatePlayer(&player);
		render();

		r_endFrame();

		QueryPerformanceCounter(&endPerformanceCount);
		double frameTime = (double)(endPerformanceCount.QuadPart - startPerformanceCount.QuadPart) / cpuFrequency;
		if (MAX_FRAME_TIME > frameTime)
			Sleep(MAX_FRAME_TIME - frameTime);

	}

	return msg.wParam;
}

Object createBox(float x, float y)
{
	Object box = createObject(
		OBJECT_BOX,
		32.0f, 32.0f,
		x, y,
		0.0f,
		1.0f, 1.0f,
		3
	);
	box.hitbox = createHitbox(32.0f, 32.0f, 22, 0, POSITION_CENTERED);
	box.renderResources.textureDisplacement[0] = 1.0f;
	box.renderResources.textureDisplacement[1] = 1.0f;
	box.renderResources.textureDisplacement[2] = 0.0f;
	box.renderResources.textureDisplacement[3] = 0.0f;

	return box;
}

Object createObject(
	ObjectType type,
	float width,
	float height,
	float x,
	float y,
	float rotation,
	float xScale,
	float yScale,
	uint32_t textureIndex)
{
	Object object = { 0 };
	object.type = type;
	object.position.x = x;
	object.position.y = y;
	object.scale.x = xScale;
	object.scale.y = yScale;
	object.rotation = rotation;
	object.renderResources = r_createObjectRenderResources(width, height, textureIndex);
	object.renderResources.displacementMatrix = multiply_m3(
		camera.vp,
		createTranslationMatrix(object.position.x, object.position.y, 1.0f),
		createMatrixSetDiagonals(xScale, yScale, 1.0f, 1.0f)
	);

	return object;
}

Player createPlayer(float x, float y)
{
	Player player = { 0 };
	player.renderResources = r_createObjectRenderResources(32.0f, 32.0f, 0);
	player.hitbox = createHitbox(24.0f, 24.0f, 1, 0, POSITION_CENTERED);
	player.position.x = x;
	player.position.y = y;
	player.rotation = 0.0f;
	player.state = STATE_GROUNDED;
	player.velocity.x = 0.0f;
	player.velocity.y = 0.0f;
	player.scale.x = 1.0f;
	player.scale.y = 1.0f;
	player.startAcceleration = 0.2f;
	player.constantSpeedBreakpoint = 0.8f;
	player.airAcceleration = 0.1f;
	player.jumpVelocity = 3.25f;
	player.maxVelocity.x = 1.3f;
	player.maxVelocity.y = 10.0f;

	player.renderResources.textureDisplacement[0] = 32.0f / 160.0f;
	player.renderResources.textureDisplacement[1] = 32.0f / 64.0f;
	player.renderResources.textureDisplacement[2] = 0.0f;
	player.renderResources.textureDisplacement[3] = 0.0f;

	return player;
}

Hitbox createHitbox(float width, float height, uint32_t colorIndex, uint32_t collisionColorIndex, Position position)
{
	Hitbox hitbox = { 0 };

	hitbox.renderResources = r_createObjectRenderResources(width, height, 2);
	hitbox.renderResources.textureDisplacement[0] = 1.0f / 9.0f;
	hitbox.renderResources.textureDisplacement[1] = 1.0f;
	hitbox.renderResources.textureDisplacement[2] = (float)colorIndex;
	hitbox.renderResources.textureDisplacement[3] = 0.0f;

	switch (position)
	{
	case POSITION_CENTERED:
	default:
		hitbox.rectangle.bottomLeft.x = -width / 2.0f;
		hitbox.rectangle.bottomLeft.y = -height / 2.0f;
		hitbox.rectangle.bottomRight.x = width / 2.0f;
		hitbox.rectangle.bottomRight.y = -height / 2.0f;
		hitbox.rectangle.topLeft.x = -width / 2.0f;
		hitbox.rectangle.topLeft.y = height / 2.0f;
		hitbox.rectangle.topRight.x = width / 2.0f;
		hitbox.rectangle.topRight.y = height / 2.0f;
		break;
	}
	hitbox.colorIndex = colorIndex;
	hitbox.collisionColorIndex = collisionColorIndex;

	return hitbox;
}

void updateBackground()
{
	Matrix4x4 trsMatrix = multiply_m3(
		createTranslationMatrix(camera.xPosition, camera.yPosition, 1.0f),
		createRotationMatrix2D(degreesToRadians(background.rotation)),
		createMatrixSetDiagonals(background.scale.x, background.scale.y, 1.0f, 1.0f)
	);

	background.renderResources.displacementMatrix = multiply_m2(
		camera.vp,
		trsMatrix
	);
}

void updatePlayer(Player *player)
{
	static uint32_t counter = 0;
	static float animationRow = 0.0f;
	static float animationCol = 0.0f;

	PlayerStates newState = player->state;

	// TODO: Animation
	switch (player->state)
	{
	case STATE_GROUNDED:
	{
		if (keyboardStates.RIGHT)
		{
			if (player->velocity.x > player->constantSpeedBreakpoint)
				player->velocity.x = player->maxVelocity.x;
			else
				player->velocity.x += player->startAcceleration;
			animationRow = 0.0f;
			if (counter < 8.0f)
				animationCol = 0.0f;
			if (counter > 8.0f)
				animationCol = 1.0f;
			if (counter > 25.0f)
				animationCol = 0.0f;
			if (counter > 35)
				animationCol = 2.0f;	
		}
		else if (keyboardStates.LEFT)
		{
			if (player->velocity.x > player->constantSpeedBreakpoint)
				player->velocity.x = player->maxVelocity.x;
			else
				player->velocity.x -= player->startAcceleration;
			animationRow = 1.0f;
		}
		else
		{
			player->velocity.x = 0.0f;
			animationCol = 0.0f;
		}
		if (keyboardStates.SPACE)
		{
			player->velocity.y += player->jumpVelocity;
			newState = STATE_JUMPING;
		}
		break;
	}
	case STATE_JUMPING:
	{
		if (keyboardStates.RIGHT)
		{
			player->velocity.x += player->airAcceleration;
		}
		else if (keyboardStates.LEFT)
		{
			player->velocity.x -= player->airAcceleration;
		}

		break;
	}
	case STATE_FALLING:
	{

		if (keyboardStates.RIGHT)
		{
			player->velocity.x += player->airAcceleration;
		}
		else if (keyboardStates.LEFT)
		{
			player->velocity.x -= player->airAcceleration;
		}
		break;
	}
	default:
		break;
	}

	player->velocity.y -= 0.19f;
	clamp(-player->maxVelocity.x, player->maxVelocity.x, &player->velocity.x);
	clamp(-player->maxVelocity.y, player->maxVelocity.y, &player->velocity.y);

	Vector2 attemptedPosition = { player->position.x + player->velocity.x, player->position.y + player->velocity.y };
	Vector2 adjustmentVector = { 0.0f, 0.0f };

	for (uint32_t i = 0; i < objectGroupCount; i++)
	{
		uint32_t updatedPosition = 0;
		if (rectanglesIntersect(player->hitbox.rectangle, player->position, objectGroups[i].hitbox.rectangle, objectGroups[i].position, NULL))
		{
			for (uint32_t j = 0; j < objectGroups[i].objectCount; j++)
			{
				attemptedPosition.x = player->position.x + player->velocity.x;
				attemptedPosition.y = player->position.y + player->velocity.y;
				adjustmentVector.x = 0.0f;
				adjustmentVector.y = 0.0f;
				// TODO: fix
				Vector2 objectPosition = { objectGroups[i].objects[j].position.x, objectGroups[i].objects[j].position.y };
				Rect objectRectangle = objectGroups[i].objects[j].hitbox.rectangle;
				// TODO: this can be simplified
				if (rectanglesIntersect(player->hitbox.rectangle, attemptedPosition, objectRectangle, objectPosition, &adjustmentVector))
				{
					Vector2 horizontalTest = { attemptedPosition.x + adjustmentVector.x, attemptedPosition.y };
					Vector2 verticalTest = { attemptedPosition.x, attemptedPosition.y + adjustmentVector.y };
					// If either of the two tests results in a new collision, discard it
					// Actual case: one axis completely overlapped
					// TODO: Handle the case where entire hitbox is overlapped
					if (rectanglesIntersect(player->hitbox.rectangle, horizontalTest, objectRectangle, objectPosition, NULL))
					{
						adjustmentVector.x = 0.0f;
					}
					else if (rectanglesIntersect(player->hitbox.rectangle, verticalTest, objectRectangle, objectPosition, NULL))
					{
						adjustmentVector.y = 0.0f;
					}
					else
					{
						// Distance from original position to horizontally adjusted potential new position
						float horizontalDistance =
							(horizontalTest.x - player->position.x) *
							(horizontalTest.x - player->position.x) +
							(horizontalTest.y - player->position.y) *
							(horizontalTest.y - player->position.y);

						// Distance from original position to vertically adjusted potential new position
						float verticalDistance =
							(verticalTest.x - player->position.x) *
							(verticalTest.x - player->position.x) +
							(verticalTest.y - player->position.y) *
							(verticalTest.y - player->position.y);

						// Adjust the new position in the direction closest to the original position
						if (horizontalDistance > verticalDistance)
							adjustmentVector.x = 0.0f;
						else
							adjustmentVector.y = 0.0f;

					}

					// If adjusted in positive y, the player has touched the ground
					if (adjustmentVector.y > 0.0f)
					{
						newState = STATE_GROUNDED;
						player->velocity.y = 0.0f;
					}
					// If adjusted in negative y, the player has touched the ceiling
					else if (adjustmentVector.y < 0.0f)
					{
						newState = STATE_FALLING;
						player->velocity.y = 0.0f;
					}
					// If adjusted in x, the player has touched a wall
					else
					{
						//player->velocity.x = 0.0f;
					}
					//player->position.x = attemptedPosition.x + adjustmentVector.x;
					//player->position.y = attemptedPosition.y + adjustmentVector.y;
					//updatedPosition = 1;
				}
			}
		}
		//if (updatedPosition == 0)
		//{
		//	player->position.x = attemptedPosition.x + adjustmentVector.x;
		//	player->position.y = attemptedPosition.y + adjustmentVector.y;
		//}

	}

	if (player->position.x == attemptedPosition.x + adjustmentVector.x)
		player->velocity.x = 0.0f;

	player->position.x = attemptedPosition.x + adjustmentVector.x;
	player->position.y = attemptedPosition.y + adjustmentVector.y;

	if (player->state != newState || counter > 45)
		counter = 0;
	else
		counter++;
	player->state = newState;

	Matrix4x4 playerTRSMatrix = multiply_m3(
		createTranslationMatrix(player->position.x, player->position.y, 1.0f),
		createRotationMatrix2D(degreesToRadians(player->rotation)),
		createMatrixSetDiagonals(player->scale.x, player->scale.y, 1.0f, 1.0f)
	);

	player->renderResources.textureDisplacement[2] = animationCol;
	player->renderResources.textureDisplacement[3] = animationRow;

	player->renderResources.displacementMatrix = multiply_m2(
		camera.vp,
		playerTRSMatrix
	);

	player->hitbox.renderResources.displacementMatrix = multiply_m2(
		camera.vp,
		playerTRSMatrix
	);
}

void updateCamera()
{
	Matrix4x4 lookAtMatrix = lookAt(
		createVector4(camera.xPosition, camera.yPosition, 0.0f, 1.0f),
		createVector4(0.0f, 0.0f, 1.0f, 1.0f),
		createVector4(0.0f, -1.0f, 0.0f, 1.0f)
	);
	lookAtMatrix = multiply_m2(createRotationMatrix2D(degreesToRadians(camera.rotation)), lookAtMatrix);
	camera.vp = multiply_m2(camera.projection, lookAtMatrix); // TODO: this aint right
}

void render()
{
	r_renderObject(background.renderResources);

	for (uint32_t i = 0; i < objectGroupCount; i++)
	{
		for (uint32_t j = 0; j < objectGroups[i].objectCount; j++)
		{
			r_renderObject(objectGroups[i].objects[j].renderResources);
		}
	}

	r_renderObject(player.renderResources);
	r_renderObject(player.hitbox.renderResources);

	for (uint32_t i = 0; i < objectGroupCount; i++)
	{
		Matrix4x4 trsMatrix = multiply_m3(
			createTranslationMatrix(objectGroups[i].position.x, objectGroups[i].position.y, 1.0f),
			createRotationMatrix2D(degreesToRadians(0.0f)),
			createMatrixSetDiagonals(1.0f, 1.0f, 1.0f, 1.0f)
		);

		objectGroups[i].hitbox.renderResources.displacementMatrix = multiply_m2(
			camera.vp,
			trsMatrix
		);

		r_renderObject(objectGroups[i].hitbox.renderResources);
	}
}

void messagePump(MSG msg, HWND window)
{
	if (PeekMessage(&msg, window, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		boolean wasDown = ((msg.lParam & (1 << 30)) != 0);
		boolean isDown = ((msg.lParam & (1 << 31)) == 0);

		switch (msg.message)
		{
		case WM_KEYDOWN:
		case WM_KEYUP:
			if (isDown && !wasDown)
				translateInput(msg.wParam, 1);
			else if (wasDown && !isDown)
				translateInput(msg.wParam, 0);
			break;
		}
	}
}

void translateInput(uint32_t key, uint32_t state)
{
	switch (key)
	{
	case 'W':
		keyboardStates.W = state;
		break;
	case 'S':
		keyboardStates.S = state;
		break;
	case 'A':
		keyboardStates.A = state;
		break;
	case 'D':
		keyboardStates.D = state;
		break;
	case 'E':
		keyboardStates.E = state;
		break;
	case 'Q':
		keyboardStates.Q = state;
		break;
	case 'F':
		keyboardStates.F = state;
		break;
	case VK_LEFT:
		keyboardStates.LEFT = state;
		break;
	case VK_RIGHT:
		keyboardStates.RIGHT = state;
		break;
	case VK_SPACE:
		keyboardStates.SPACE = state;
		break;
	default:
		break;
	}
}

void handleInput()
{
	if (keyboardStates.W)
		camera.yPosition -= camera.speed;
	if (keyboardStates.S)
		camera.yPosition += camera.speed;
	if (keyboardStates.A)
		camera.xPosition += camera.speed;
	if (keyboardStates.D)
		camera.xPosition -= camera.speed;
	if (keyboardStates.Q)
		camera.rotation += 1.0f;
	if (keyboardStates.E)
		camera.rotation -= 1.0f;
	if (keyboardStates.F)
		camera.rotation = 0.0f;
}