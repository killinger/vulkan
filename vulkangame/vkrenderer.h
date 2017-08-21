#pragma once
#include "simd_math.h"
typedef struct Vertex
{
	float position[2];
	float color[3];
	float textureCoordinates[2];
} Vertex;

typedef struct Quad
{
	Vertex vertices[6];
} Quad;

typedef struct Texture
{
	uint32_t width;
	uint32_t height;
	VkImage image;
	VkImageView imageView;
	VkDeviceMemory imageMemory;
	VkSampler sampler;
} Texture;

typedef struct ObjectRenderResources
{
	uint32_t textureIndex;
	VkDescriptorSet descriptorSet;
	uint32_t vertexCount;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer uniformDisplacementBuffer;
	VkDeviceMemory uniformDisplacementBufferMemory;
	VkDeviceSize uniformDisplacementBufferSize;
	VkBuffer uniformTextureDisplacementBuffer;
	VkDeviceMemory uniformTextureDisplacementBufferMemory;
	VkDeviceSize uniformTextureDisplacementBufferSize;
	float textureDisplacement[4];
	Matrix4x4 displacementMatrix;
} ObjectRenderResources;

typedef struct PresentationInfo
{
	VkSurfaceKHR surface;
	VkPresentModeKHR presentMode;
	VkSurfaceFormatKHR surfaceFormat;
	VkExtent2D extent;	
	uint32_t imagesCount;
	VkImage *images;
	VkImageView *imageViews;
	uint32_t nextImageIndex;
} PresentationInfo;

typedef struct ShaderInfo
{
	size_t codeSize;
	char *shaderCode;
} ShaderInfo;

HWND r_initVkRenderer(HINSTANCE hInstance, int nShowCmd);
void r_beginFrame();
void r_endFrame();
ObjectRenderResources r_createObjectRenderResources(float width, float height, uint32_t textureIndex);
void r_renderObject(ObjectRenderResources renderResources);