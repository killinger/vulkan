#pragma once
HWND r_initVkRenderer(HINSTANCE hInstance, int nShowCmd);

typedef struct PresentationInfo
{
	VkSurfaceKHR surface;
	VkPresentModeKHR presentMode;
	VkSurfaceFormatKHR surfaceFormat;
	VkExtent2D extent;	
	uint32_t imagesCount;
	VkImage *images;
	VkImageView *imageViews;
} PresentationInfo;

typedef struct ShaderInfo
{
	size_t codeSize;
	char *shaderCode;
} ShaderInfo;

typedef struct Vertex
{
	float position[2];
	float color[4];
	float textureCoordinates[2];
} Vertex;

typedef struct Quad
{
	Vertex vertices[6];
} Quad;