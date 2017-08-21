#define VK_USE_PLATFORM_WIN32_KHR

#include <Windows.h>
#include <vulkan\vulkan.h>
#include <stdio.h>
#include <tchar.h>
#include <assert.h>
#include "simd_math.h"
#include "vkrenderer.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// TODO: Multithreading scheme
//		 * Queues
//		 * Command buffers
//		 * Synchronization primitives
// TODO: Proper resolution setting
// TODO: Game loop
// TODO: Reasonable struct layout
// TODO: Check which surface formats must be supported

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

// file scope function declarations
static VkInstance r_createInstance();
static VkPhysicalDevice r_findFirstCompatibleDevice(VkInstance instance);
static VkDevice r_createLogicalDevice(
	VkPhysicalDevice physicalDevice,
	uint32_t queueFamiliesCount,
	VkQueueFamilyProperties *queueFamiliesProperties);
static VkSurfaceKHR r_createPresentationSurface(
	VkInstance instance,
	HINSTANCE hInstance,
	HWND window);
static VkSwapchainKHR r_createSwapChain(
	VkDevice device,
	VkSurfaceKHR presentationSurface,
	uint32_t minImageCount,
	VkSurfaceFormatKHR surfaceFormat,
	VkExtent2D extent,
	VkPresentModeKHR presentMode,
	VkSwapchainKHR oldSwapchain);
static VkCommandPool r_createCommandPool(
	VkDevice device,
	VkCommandPoolCreateFlags createFlags,
	uint32_t queueFamilyIndex);
static VkCommandBuffer *r_allocateCommandBuffers(
	VkDevice device,
	VkCommandPool commandPool,
	VkCommandBufferLevel commandBufferLevel,
	uint32_t commandBufferCount);
static VkShaderModule r_createShaderModule(
	VkDevice device,
	size_t codeSize,
	const uint32_t *code);
static VkRenderPass r_createRenderPass(
	VkDevice device,
	VkFormat format);
static VkPipelineLayout r_createPipelineLayout(
	VkDevice device,
	uint32_t descriptorSetLayoutCount,
	VkDescriptorSetLayout *descriptorSetLayouts,
	uint32_t pushConstantRangeCount,
	VkPushConstantRange *pushConstantRanges);
static VkPipeline r_createGraphicsPipeline(
	VkDevice device,
	uint32_t shaderStageCount,
	VkShaderStageFlags *shaderStages,
	VkShaderModule *shaderModules,
	uint32_t vertexBindingDescriptionCount,
	VkVertexInputBindingDescription *vertexBindingDescriptions,
	uint32_t vertexAttributeDescriptionCount,
	VkVertexInputAttributeDescription *vertexAttributeDescriptions,
	uint32_t viewportCount,
	VkViewport *viewports,
	uint32_t scissorCount,
	VkRect2D *scissors,
	VkPipelineLayout pipelineLayout,
	VkRenderPass renderPass);
static VkFramebuffer r_createFramebuffer(
	VkDevice device,
	VkRenderPass renderPass,
	uint32_t imageViewCount,
	VkImageView imageView,
	VkExtent2D extent);
static VkBuffer r_createBuffer(
	VkDevice device,
	VkDeviceSize bufferSize,
	VkBufferUsageFlags usageFlags);
static VkDeviceMemory r_allocateBufferMemory(
	VkDevice device,
	VkBuffer buffer,
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties,
	VkMemoryPropertyFlags memoryPropertyFlags);
static void r_copyDataToBuffer(
	VkDevice device,
	size_t dataSize,
	void *data,
	VkDeviceMemory bufferMemory,
	VkDeviceSize offset);
static VkImageView r_createImageView(
	VkDevice device,
	VkImage image,
	VkImageViewType viewType,
	VkFormat format);
static VkDeviceMemory r_allocateImageMemory(
	VkDevice device,
	VkImage image,
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties,
	VkMemoryPropertyFlags memoryPropertyFlags);
static void r_copyBufferToImage(
	uint32_t width,
	uint32_t height,
	VkBuffer buffer,
	VkImage image);
static VkSemaphore r_createSemaphore(VkDevice device);
static VkDescriptorSetLayout r_createDescriptorSetLayout(
	VkDevice device,
	uint32_t bindingCount,
	VkDescriptorSetLayoutBinding  *descriptorSetLayoutBindings);


static int32_t r_findQueueFamilyIndex(
	uint32_t queueFamilyCount,
	VkQueueFamilyProperties *queueFamilyProperties,
	VkQueueFlags desiredCapabilites);
static int32_t r_findCapableMemoryIndex(
	VkMemoryRequirements memoryRequirements,
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties,
	VkMemoryPropertyFlags memoryPropertyFlags);
static boolean r_isExtensionAvailable(
	const char* const desiredExtension,
	uint32_t availableExtensionsCount,
	VkExtensionProperties *availableExtensions);
static VkPresentModeKHR r_getBestMatchPresentMode(
	VkPhysicalDevice physicalDevice,
	VkSurfaceKHR presentationSurface,
	uint32_t desiredPresentModesCount,
	VkPresentModeKHR *desiredPresentModes);
static boolean r_isPresentModeAvailable(
	VkPresentModeKHR desiredPresentMode,
	uint32_t availablePresentModesCount,
	VkPresentModeKHR *availablePresentModes);
static VkSurfaceFormatKHR r_getBestMatchSurfaceFormat(
	VkPhysicalDevice physicalDevice,
	VkSurfaceKHR presentationSurface,
	uint32_t desiredSurfaceFormatsCount,
	VkSurfaceFormatKHR *desiredSurfaceFormats);
static boolean r_isSurfaceFormatAvailable(
	VkSurfaceFormatKHR desiredSurfaceFormat,
	uint32_t availableSurfaceFormatsCount,
	VkSurfaceFormatKHR *availableSurfaceFormats);



static Quad r_createQuad(float width, float height);
static void r_transitionTextureImageLayout(
	VkImage image,
	VkAccessFlags oldAccessMask,
	VkAccessFlags newAccessMask,
	VkImageLayout oldLayout,
	VkImageLayout newLayout);
static void r_transitionImageLayout(
	VkAccessFlags sourceAccessMask,
	VkAccessFlags desinationAccessMask,
	VkImageLayout oldLayout,
	VkImageLayout newLayout,
	uint32_t sourceQueueFamilyIndex,
	uint32_t destinationQueueFamilyIndex,
	VkImage image,
	VkImageAspectFlags imageAspectMask);
static VkCommandBuffer r_beginOneShotRecording(
	VkDevice device,
	VkCommandPool commandPool);
static void r_endOneShotRecording(
	VkDevice device,
	VkCommandPool commandPool,
	VkCommandBuffer commandBuffer);
static void r_bindVertexBuffers(uint32_t firstBinding,
	uint32_t bindingCount,
	VkBuffer *vertexBuffers,
	VkDeviceSize *offsets);
static void r_createQuadVertexBufferAndMemory(
	Quad quad,
	VkBuffer* buffer,
	VkDeviceMemory* memory);
static uint32_t r_getSecondaryCommandBufferIndex();
static Texture r_createTexture(char *filename);

LRESULT CALLBACK WndProc(HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam);
static HWND r_createAndRegisterWindow(HINSTANCE hInstance);
static ShaderInfo r_readShaderFile(const char *filename);

// file scope variables
static VkInstance g_instance;
static VkPhysicalDevice g_physicalDevice;
static VkPhysicalDeviceMemoryProperties g_physicalDeviceMemoryProperties;
static VkDevice g_device;
static VkQueue g_graphicsQueue;
static PresentationInfo g_presentationInfo;
static VkSwapchainKHR g_swapchain;

static VkCommandPool g_commandPool;
static VkCommandBuffer *g_commandBuffers;
static VkCommandBuffer *g_commandBuffersSecondary;
static const uint32_t g_commandBuffersSecondaryCount = 64;
static uint32_t g_commandBuffersSecondaryActive = 0;

static VkPipeline g_graphicsPipeline;
static VkPipelineLayout g_pipelineLayout;
static VkRenderPass g_renderPass;
static VkFramebuffer *g_framebuffers;

static VkBuffer g_vertexBuffer;
static VkDeviceMemory g_vertexBufferMemory;
static VkBuffer g_uniformBuffer;
static VkDeviceMemory g_uniformBufferMemory;
static VkBuffer g_textureTransformBuffer;
static VkDeviceMemory g_textureTransformBufferMemory;

static VkDescriptorPool g_descriptorPool;
static VkDescriptorSetLayout g_descriptorSetLayout;
static uint32_t g_descriptorSetCount;
static VkDescriptorSet *g_descriptorSets;
static VkSampler sampler;
VkImageView textureImageView;


Texture *g_textures;
uint32_t g_textureCount;

static VkSemaphore g_imageAcquiredSemaphore;

// shaders
static ShaderInfo g_vertexShaderInfo;
static VkShaderModule g_vertexShader;
static ShaderInfo g_fragmentShaderInfo;
static VkShaderModule g_fragmentShader;


// test test test

static float UNIFORM_ROTATION = 0.0f;
static float UNIFORM_X_POSITION = 0.0f;
static float UNIFORM_Y_POSITION = 0.0f;
static float UNIFORM_X_SCALE = 1.0f;
static float UNIFORM_Y_SCALE = 1.0f;

Matrix4x4 orthographicProjectionMatrix;
Matrix4x4 cameraBasis;

uint32_t textureCount = 4;
Texture *textures;


//

// function definitions
HWND r_initVkRenderer(HINSTANCE hInstance, int nShowCmd)
{
	VkResult result;

	HWND window = r_createAndRegisterWindow(hInstance);
	ShowWindow(window, nShowCmd);

	// INSTANCE
	g_instance = r_createInstance();

	// PHYSICAL DEVICE
	g_physicalDevice = r_findFirstCompatibleDevice(g_instance);
	vkGetPhysicalDeviceMemoryProperties(g_physicalDevice, &g_physicalDeviceMemoryProperties);

	// QUEUES & LOGICAL DEVICE
	// TODO: It's a mess. Needs a scheme.
	uint32_t queueFamiliesCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(g_physicalDevice, &queueFamiliesCount, NULL);
	assert(queueFamiliesCount > 0);

	VkQueueFamilyProperties *queueFamiliesProperties =
		(VkQueueFamilyProperties*)malloc(queueFamiliesCount * sizeof(VkQueueFamilyProperties));
	vkGetPhysicalDeviceQueueFamilyProperties(g_physicalDevice, &queueFamiliesCount, queueFamiliesProperties);

	g_device = r_createLogicalDevice(g_physicalDevice, queueFamiliesCount, queueFamiliesProperties);

	uint32_t graphicsQueueFamilyIndex = r_findQueueFamilyIndex(queueFamiliesCount, queueFamiliesProperties, VK_QUEUE_GRAPHICS_BIT);
	vkGetDeviceQueue(g_device, graphicsQueueFamilyIndex, 0, &g_graphicsQueue);

	// PRESENTATION SURFACE
	g_presentationInfo.surface = r_createPresentationSurface(g_instance, hInstance, window);
	VkBool32 surfaceIsSupported;
	vkGetPhysicalDeviceSurfaceSupportKHR(g_physicalDevice,
		graphicsQueueFamilyIndex,
		g_presentationInfo.surface,
		&surfaceIsSupported);
	assert(surfaceIsSupported == VK_TRUE);

	uint32_t desiredPresentModesCount = 1;
	VkPresentModeKHR desiredPresentModes[] =
	{
		VK_PRESENT_MODE_MAILBOX_KHR
	};
	g_presentationInfo.presentMode = r_getBestMatchPresentMode(g_physicalDevice,
		g_presentationInfo.surface,
		desiredPresentModesCount,
		desiredPresentModes);
	uint32_t desiredSurfaceFormatsCount = 1;
	VkSurfaceFormatKHR desiredSurfaceFormats[] =
	{
		{.format = VK_FORMAT_B8G8R8A8_UNORM,.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }
	};
	g_presentationInfo.surfaceFormat = r_getBestMatchSurfaceFormat(g_physicalDevice,
		g_presentationInfo.surface,
		desiredSurfaceFormatsCount,
		desiredSurfaceFormats);

	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_physicalDevice, g_presentationInfo.surface, &surfaceCapabilities);
	assert(result == VK_SUCCESS);

	uint32_t minPresentationImagesCount = surfaceCapabilities.minImageCount + 1;
	if (surfaceCapabilities.maxImageCount > 0)
	{
		if (minPresentationImagesCount > surfaceCapabilities.maxImageCount)
			minPresentationImagesCount = surfaceCapabilities.maxImageCount;
	}

	// TODO: Voodoo
	g_presentationInfo.extent = surfaceCapabilities.currentExtent;

	// SWAPCHAIN
	g_swapchain = r_createSwapChain(g_device,
		g_presentationInfo.surface,
		minPresentationImagesCount,
		g_presentationInfo.surfaceFormat,
		g_presentationInfo.extent,
		g_presentationInfo.presentMode,
		VK_NULL_HANDLE);
	result = vkGetSwapchainImagesKHR(g_device, g_swapchain, &g_presentationInfo.imagesCount, NULL);
	assert(result == VK_SUCCESS);

	g_presentationInfo.images =
		(VkImage*)malloc(g_presentationInfo.imagesCount * sizeof(VkImage)); // TODO: free

	result = vkGetSwapchainImagesKHR(g_device, g_swapchain, &g_presentationInfo.imagesCount, g_presentationInfo.images);
	assert(result == VK_SUCCESS);

	g_presentationInfo.imageViews =
		(VkImageView*)malloc(g_presentationInfo.imagesCount * sizeof(VkImageView)); // TODO: free

	for (uint32_t i = 0; i < g_presentationInfo.imagesCount; i++)
	{
		g_presentationInfo.imageViews[i] = r_createImageView(g_device,
			g_presentationInfo.images[i],
			VK_IMAGE_VIEW_TYPE_2D,
			g_presentationInfo.surfaceFormat.format);
	}

	// COMMAND POOL & BUFFER
	g_commandPool = r_createCommandPool(g_device, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, graphicsQueueFamilyIndex);
	g_commandBuffers = r_allocateCommandBuffers(g_device, g_commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, g_presentationInfo.imagesCount);
	g_commandBuffersSecondary = r_allocateCommandBuffers(g_device, g_commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY, g_commandBuffersSecondaryCount);

	// SHADERS
	// TODO: Destroy after pipeline creation?
	g_vertexShaderInfo = r_readShaderFile("../vulkangame/vert.spv");
	g_fragmentShaderInfo = r_readShaderFile("../vulkangame/frag.spv");

	g_vertexShader = r_createShaderModule(g_device, g_vertexShaderInfo.codeSize, (const uint32_t*)g_vertexShaderInfo.shaderCode);
	g_fragmentShader = r_createShaderModule(g_device, g_fragmentShaderInfo.codeSize, (const uint32_t*)g_fragmentShaderInfo.shaderCode);

	// VERTEX BUFFER
	Quad quad = r_createQuad(50.0f, 50.0f);
	r_createQuadVertexBufferAndMemory(quad, &g_vertexBuffer, &g_vertexBufferMemory);

	// UNIFORM BUFFER
	orthographicProjectionMatrix =
		createOrthographicProjectionMatrix(-1280.0f / 2, 1280.0f / 2, -720.0f / 2, 720.0f / 2, 0.1f, 10.0f);
	cameraBasis = createMatrixSetDiagonals(1.0f, -1.0f, -1.0f, 1.0f);

	Matrix4x4 modelMatrix = multiply_m3(
		createTranslationMatrix(0.0f, 0.0f, 0.0f),
		createRotationMatrix2D(degreesToRadians(0.0f)),
		createScalingMatrix2D(1.0f, 1.0f)
	);

	Matrix4x4 mvpMatrix = multiply_m3(orthographicProjectionMatrix, cameraBasis, modelMatrix);

	g_uniformBuffer = r_createBuffer(g_device, sizeof(Matrix4x4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	g_uniformBufferMemory = r_allocateBufferMemory(
		g_device,
		g_uniformBuffer,
		g_physicalDeviceMemoryProperties,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	result = vkBindBufferMemory(g_device, g_uniformBuffer, g_uniformBufferMemory, 0);
	assert(result == VK_SUCCESS);

	r_copyDataToBuffer(g_device, sizeof(Matrix4x4), &mvpMatrix, g_uniformBufferMemory, 0);

	// TEXTURE TRANSFORMATION
	float testest[4];
	testest[0] = 64.0f / 128.0f;
	testest[1] = 64.0f / 128.0f;
	testest[2] = 0.0f;
	testest[3] = 0.0f;

	g_textureTransformBuffer = r_createBuffer(g_device, sizeof(testest), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	g_textureTransformBufferMemory = r_allocateBufferMemory(
		g_device,
		g_textureTransformBuffer,
		g_physicalDeviceMemoryProperties,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	result = vkBindBufferMemory(g_device, g_textureTransformBuffer, g_textureTransformBufferMemory, 0);
	assert(result == VK_SUCCESS);

	r_copyDataToBuffer(g_device, sizeof(testest), &testest, g_textureTransformBufferMemory, 0);

	// TEXTURES
	textures = (Texture*)malloc(textureCount * sizeof(Texture));
	textures[0] = r_createTexture("../resources/boxman.png");
	textures[1] = r_createTexture("../resources/doggo.jpg");
	textures[2] = r_createTexture("../resources/hitbox.png");
	textures[3] = r_createTexture("../resources/testbox.png");

	uint32_t componentsCount = 0;
	int32_t imageWidth;
	int32_t imageHeight;
	char *fileName = "../resources/boxman.png";

	unsigned char *stbiData = stbi_load(fileName, &imageWidth, &imageHeight, &componentsCount, STBI_rgb_alpha);
	assert(componentsCount > 0);
	size_t dataSize = imageWidth * imageHeight * 4;

	VkDeviceSize imageSize = dataSize;

	VkBuffer stagingBuffer = r_createBuffer(g_device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	VkDeviceMemory stagingBufferMemory = r_allocateBufferMemory(
		g_device,
		stagingBuffer,
		g_physicalDeviceMemoryProperties,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	result = vkBindBufferMemory(g_device, stagingBuffer, stagingBufferMemory, 0);
	assert(result == VK_SUCCESS);
	r_copyDataToBuffer(g_device, imageSize, stbiData, stagingBufferMemory, 0);
	stbi_image_free(stbiData);

	VkImageCreateInfo textureImageCreateInfo = { 0 };
	textureImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	textureImageCreateInfo.pNext = NULL;
	textureImageCreateInfo.flags = 0;
	textureImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	textureImageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	textureImageCreateInfo.extent.width = (uint32_t)imageWidth;
	textureImageCreateInfo.extent.height = (uint32_t)imageHeight;
	textureImageCreateInfo.extent.depth = 1;
	textureImageCreateInfo.mipLevels = 1;
	textureImageCreateInfo.arrayLayers = 1;
	textureImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	textureImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	textureImageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	textureImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	textureImageCreateInfo.queueFamilyIndexCount = 0;
	textureImageCreateInfo.pQueueFamilyIndices = NULL;
	textureImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

	VkImage textureImage;
	result = vkCreateImage(g_device, &textureImageCreateInfo, NULL, &textureImage);
	assert(result == VK_SUCCESS);

	VkDeviceMemory textureImageMemory = r_allocateImageMemory(
		g_device, textureImage,
		g_physicalDeviceMemoryProperties,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	result = vkBindImageMemory(g_device, textureImage, textureImageMemory, 0);
	assert(result == VK_SUCCESS);

	r_transitionTextureImageLayout(
		textureImage,
		VK_ACCESS_HOST_WRITE_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	r_copyBufferToImage(imageWidth, imageHeight, stagingBuffer, textureImage);
	r_transitionTextureImageLayout(
		textureImage,
		VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	textureImageView = r_createImageView(
		g_device,
		textureImage,
		VK_IMAGE_VIEW_TYPE_2D,
		VK_FORMAT_R8G8B8A8_UNORM);

	VkSamplerCreateInfo samplerCreateInfo = { 0 };
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.pNext = NULL;
	samplerCreateInfo.flags = 0;
	samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
	samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.anisotropyEnable = VK_FALSE;
	samplerCreateInfo.maxAnisotropy = 1;
	samplerCreateInfo.compareEnable = VK_FALSE;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = 0.0f;
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

	
	result = vkCreateSampler(g_device, &samplerCreateInfo, NULL, &sampler);
	assert(result == VK_SUCCESS);

	// - - - DESCRIPTORS + RESOURCES
	uint32_t descriptorSetLayoutBindingCount = 3;
	VkDescriptorSetLayoutBinding descriptorSetLayoutBindings[] =
	{
		{
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.pImmutableSamplers = NULL
		},
		{
			.binding = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = NULL
		},
		{
			.binding = 2,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.pImmutableSamplers = NULL
		}
	};
	g_descriptorSetLayout = r_createDescriptorSetLayout(
		g_device,
		descriptorSetLayoutBindingCount,
		descriptorSetLayoutBindings);

	uint32_t descriptorPoolSizeCount = 2;
	VkDescriptorPoolSize descriptorPoolSizes[] =
	{
		{.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,.descriptorCount = 256 },
		{.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,.descriptorCount = 128 }
	};
	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = { 0 };
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.pNext = NULL;
	descriptorPoolCreateInfo.flags = 0;
	descriptorPoolCreateInfo.maxSets = 128;
	descriptorPoolCreateInfo.poolSizeCount = descriptorPoolSizeCount;
	descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes;

	result = vkCreateDescriptorPool(g_device, &descriptorPoolCreateInfo, NULL, &g_descriptorPool);
	assert(result == VK_SUCCESS);

	g_descriptorSetCount = 1;
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = { 0 };
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.pNext = NULL;
	descriptorSetAllocateInfo.descriptorPool = g_descriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = g_descriptorSetCount;
	descriptorSetAllocateInfo.pSetLayouts = &g_descriptorSetLayout;

	g_descriptorSets = (VkDescriptorSet*)malloc(g_descriptorSetCount * sizeof(VkDescriptorSet));
	result = vkAllocateDescriptorSets(g_device, &descriptorSetAllocateInfo, g_descriptorSets);
	assert(result == VK_SUCCESS);

	VkDescriptorBufferInfo descriptorBufferInfo = { 0 };
	descriptorBufferInfo.buffer = g_uniformBuffer;
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range = sizeof(Matrix4x4);

	VkDescriptorBufferInfo descriptorTextureTransformBufferInfo = { 0 };
	descriptorTextureTransformBufferInfo.buffer = g_textureTransformBuffer;
	descriptorTextureTransformBufferInfo.offset = 0;
	descriptorTextureTransformBufferInfo.range = sizeof(testest);

	VkDescriptorImageInfo descriptorImageInfo = { 0 };
	descriptorImageInfo.sampler = sampler;
	descriptorImageInfo.imageView = textureImageView;
	descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet writeDescriptorSet[3];
	writeDescriptorSet[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet[0].pNext = NULL;
	writeDescriptorSet[0].dstSet = *g_descriptorSets;
	writeDescriptorSet[0].dstBinding = 0;
	writeDescriptorSet[0].dstArrayElement = 0;
	writeDescriptorSet[0].descriptorCount = g_descriptorSetCount;
	writeDescriptorSet[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSet[0].pImageInfo = NULL;
	writeDescriptorSet[0].pBufferInfo = &descriptorBufferInfo;
	writeDescriptorSet[0].pTexelBufferView = NULL;

	writeDescriptorSet[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet[1].pNext = NULL;
	writeDescriptorSet[1].dstSet = *g_descriptorSets;
	writeDescriptorSet[1].dstBinding = 1;
	writeDescriptorSet[1].dstArrayElement = 0;
	writeDescriptorSet[1].descriptorCount = g_descriptorSetCount;
	writeDescriptorSet[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeDescriptorSet[1].pImageInfo = &descriptorImageInfo;
	writeDescriptorSet[1].pBufferInfo = NULL;
	writeDescriptorSet[1].pTexelBufferView = NULL;

	writeDescriptorSet[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet[2].pNext = NULL;
	writeDescriptorSet[2].dstSet = *g_descriptorSets;
	writeDescriptorSet[2].dstBinding = 2;
	writeDescriptorSet[2].dstArrayElement = 0;
	writeDescriptorSet[2].descriptorCount = g_descriptorSetCount;
	writeDescriptorSet[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSet[2].pImageInfo = NULL;
	writeDescriptorSet[2].pBufferInfo = &descriptorTextureTransformBufferInfo;
	writeDescriptorSet[2].pTexelBufferView = NULL;

	vkUpdateDescriptorSets(
		g_device,
		3, writeDescriptorSet,
		0, NULL);

	// RENDER PASS
	g_renderPass = r_createRenderPass(g_device, g_presentationInfo.surfaceFormat.format);

	// GRAPHICS PIPELINE
	g_pipelineLayout = r_createPipelineLayout(g_device,
		g_descriptorSetCount, &g_descriptorSetLayout,
		0, NULL);

	uint32_t shaderStageCount = 2;
	VkShaderStageFlags shaderStages[] = { VK_SHADER_STAGE_VERTEX_BIT,
											VK_SHADER_STAGE_FRAGMENT_BIT };
	VkShaderModule shaderModules[] = { g_vertexShader,
										g_fragmentShader };

	uint32_t vertexInputBindingDescriptionCount = 1;
	VkVertexInputBindingDescription vertexInputBindingDescriptions[] = {
		{
			.binding = 0,
			.stride = sizeof(Vertex),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
		}
	};

	uint32_t vertexInputAttributeDescriptionCount = 3;
	VkVertexInputAttributeDescription vertexInputAttributeDescriptions[] = {
		{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(Vertex, position)
		},
		{
			.location = 1,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(Vertex, color)
		},
		{
			.location = 2,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(Vertex, textureCoordinates)

		}
	};

	uint32_t viewportCount = 1;
	VkViewport viewports[] = {
		{
			.x = 0.0f,
			.y = 0.0f,
			.width = (float)g_presentationInfo.extent.width,
			.height = (float)g_presentationInfo.extent.height,
			.minDepth = 0.0f,
			.maxDepth = 1.0f
		}
	};

	uint32_t scissorCount = 1;
	VkRect2D scissors[] = {
		{
			.offset.x = 0,
			.offset.y = 0,
			.extent = g_presentationInfo.extent
		}
	};

	g_graphicsPipeline = r_createGraphicsPipeline(g_device,
		shaderStageCount, shaderStages, shaderModules,
		vertexInputBindingDescriptionCount, vertexInputBindingDescriptions,
		vertexInputAttributeDescriptionCount, vertexInputAttributeDescriptions,
		viewportCount, viewports,
		scissorCount, scissors,
		g_pipelineLayout,
		g_renderPass);

	// FRAME BUFFERS
	g_framebuffers = (VkFramebuffer*)malloc(g_presentationInfo.imagesCount * sizeof(VkFramebuffer));
	for (uint32_t i = 0; i < g_presentationInfo.imagesCount; i++)
	{
		g_framebuffers[i] = r_createFramebuffer(g_device,
			g_renderPass,
			g_presentationInfo.imagesCount,
			g_presentationInfo.imageViews[i],
			g_presentationInfo.extent);
	}

	// SEMAPHORES
	g_imageAcquiredSemaphore = r_createSemaphore(g_device);

	return window;
}

void r_beginFrame()
{
	VkResult result;

	vkAcquireNextImageKHR(g_device, g_swapchain, 20000, g_imageAcquiredSemaphore, VK_NULL_HANDLE, &g_presentationInfo.nextImageIndex);
	vkQueueWaitIdle(g_graphicsQueue);
	
	VkCommandBufferBeginInfo commandBufferBeginInfo = { 0 };
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = NULL;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	commandBufferBeginInfo.pInheritanceInfo = NULL;

	VkClearValue clearValue = { 1.0f, 1.0f, 1.0f, 1.0f };
	
	VkRenderPassBeginInfo renderPassBeginInfo = { 0 };
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.pNext = NULL;
	renderPassBeginInfo.renderPass = g_renderPass;
	renderPassBeginInfo.framebuffer = g_framebuffers[g_presentationInfo.nextImageIndex];
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent = g_presentationInfo.extent;
	renderPassBeginInfo.clearValueCount = 1;
	renderPassBeginInfo.pClearValues = &clearValue;
	
	result = vkBeginCommandBuffer(g_commandBuffers[g_presentationInfo.nextImageIndex], &commandBufferBeginInfo);
	assert(result == VK_SUCCESS);

	vkCmdBindPipeline(g_commandBuffers[g_presentationInfo.nextImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, g_graphicsPipeline);
	vkCmdBeginRenderPass(g_commandBuffers[g_presentationInfo.nextImageIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void r_endFrame()
{
	VkResult result;
	vkCmdEndRenderPass(g_commandBuffers[g_presentationInfo.nextImageIndex]);

	result = vkEndCommandBuffer(g_commandBuffers[g_presentationInfo.nextImageIndex]);
	assert(result == VK_SUCCESS);

	VkSemaphore submitSignalSemaphore = r_createSemaphore(g_device);

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSubmitInfo submitInfo = { 0 };
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = NULL;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &g_imageAcquiredSemaphore;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &g_commandBuffers[g_presentationInfo.nextImageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &submitSignalSemaphore;

	result = vkQueueSubmit(g_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	assert(result == VK_SUCCESS);

	VkPresentInfoKHR presentInfo = { 0 };
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = NULL;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &submitSignalSemaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &g_swapchain;
	presentInfo.pImageIndices = &g_presentationInfo.nextImageIndex;
	presentInfo.pResults = NULL;

	result = vkQueuePresentKHR(g_graphicsQueue, &presentInfo);
	assert(result == VK_SUCCESS);
}

ObjectRenderResources r_createObjectRenderResources(float width, float height, uint32_t textureIndex)
{
	VkResult result;
	ObjectRenderResources objectRenderResources = { 0 };
	objectRenderResources.textureIndex = textureIndex;
	Quad quad = r_createQuad(width, height);
	
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = { 0 };
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.pNext = NULL;
	descriptorSetAllocateInfo.descriptorPool = g_descriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	descriptorSetAllocateInfo.pSetLayouts = &g_descriptorSetLayout;
	
	result = vkAllocateDescriptorSets(g_device, &descriptorSetAllocateInfo, &objectRenderResources.descriptorSet);
	assert(result == VK_SUCCESS);

	objectRenderResources.vertexCount = 6;
	r_createQuadVertexBufferAndMemory(quad, &objectRenderResources.vertexBuffer, &objectRenderResources.vertexBufferMemory);
	
	objectRenderResources.uniformDisplacementBuffer = r_createBuffer(g_device, sizeof(Matrix4x4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	objectRenderResources.uniformDisplacementBufferMemory = r_allocateBufferMemory(
		g_device,
		objectRenderResources.uniformDisplacementBuffer,
		g_physicalDeviceMemoryProperties,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	result = vkBindBufferMemory(
		g_device,
		objectRenderResources.uniformDisplacementBuffer,
		objectRenderResources.uniformDisplacementBufferMemory,
		0);
	assert(result == VK_SUCCESS);
	
	objectRenderResources.uniformTextureDisplacementBuffer = r_createBuffer(
		g_device, 
		sizeof(objectRenderResources.textureDisplacement), 
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	objectRenderResources.uniformTextureDisplacementBufferMemory = r_allocateBufferMemory(
		g_device,
		objectRenderResources.uniformTextureDisplacementBuffer,
		g_physicalDeviceMemoryProperties,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	result = vkBindBufferMemory(
		g_device,
		objectRenderResources.uniformTextureDisplacementBuffer,
		objectRenderResources.uniformTextureDisplacementBufferMemory,
		0);
	assert(result == VK_SUCCESS);

	return objectRenderResources;
}

void r_renderObject(ObjectRenderResources renderResources)
{
	VkDeviceSize offsets[] = { 0 };

	r_copyDataToBuffer(
		g_device, 
		sizeof(renderResources.displacementMatrix), 
		&renderResources.displacementMatrix, 
		renderResources.uniformDisplacementBufferMemory, 
		0);
	r_copyDataToBuffer(
		g_device,
		sizeof(renderResources.textureDisplacement),
		&renderResources.textureDisplacement,
		renderResources.uniformTextureDisplacementBufferMemory,
		0);

	VkDescriptorBufferInfo descriptorBufferInfo = { 0 };
	descriptorBufferInfo.buffer = renderResources.uniformDisplacementBuffer;
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range = sizeof(renderResources.uniformDisplacementBuffer);

	VkDescriptorBufferInfo descriptorTextureTransformBufferInfo = { 0 };
	descriptorTextureTransformBufferInfo.buffer = renderResources.uniformTextureDisplacementBuffer;
	descriptorTextureTransformBufferInfo.offset = 0;
	descriptorTextureTransformBufferInfo.range = sizeof(renderResources.textureDisplacement);

	VkDescriptorImageInfo descriptorImageInfo = { 0 };
	descriptorImageInfo.sampler = textures[renderResources.textureIndex].sampler;
	descriptorImageInfo.imageView = textures[renderResources.textureIndex].imageView;
	descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet writeDescriptorSet[3];
	writeDescriptorSet[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet[0].pNext = NULL;
	writeDescriptorSet[0].dstSet = renderResources.descriptorSet;
	writeDescriptorSet[0].dstBinding = 0;
	writeDescriptorSet[0].dstArrayElement = 0;
	writeDescriptorSet[0].descriptorCount = g_descriptorSetCount;
	writeDescriptorSet[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSet[0].pImageInfo = NULL;
	writeDescriptorSet[0].pBufferInfo = &descriptorBufferInfo;
	writeDescriptorSet[0].pTexelBufferView = NULL;

	writeDescriptorSet[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet[1].pNext = NULL;
	writeDescriptorSet[1].dstSet = renderResources.descriptorSet;
	writeDescriptorSet[1].dstBinding = 1;
	writeDescriptorSet[1].dstArrayElement = 0;
	writeDescriptorSet[1].descriptorCount = g_descriptorSetCount;
	writeDescriptorSet[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeDescriptorSet[1].pImageInfo = &descriptorImageInfo;
	writeDescriptorSet[1].pBufferInfo = NULL;
	writeDescriptorSet[1].pTexelBufferView = NULL;

	writeDescriptorSet[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet[2].pNext = NULL;
	writeDescriptorSet[2].dstSet = renderResources.descriptorSet;
	writeDescriptorSet[2].dstBinding = 2;
	writeDescriptorSet[2].dstArrayElement = 0;
	writeDescriptorSet[2].descriptorCount = g_descriptorSetCount;
	writeDescriptorSet[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSet[2].pImageInfo = NULL;
	writeDescriptorSet[2].pBufferInfo = &descriptorTextureTransformBufferInfo;
	writeDescriptorSet[2].pTexelBufferView = NULL;

	vkUpdateDescriptorSets(
		g_device,
		3, writeDescriptorSet,
		0, NULL);

	vkCmdBindVertexBuffers(g_commandBuffers[g_presentationInfo.nextImageIndex], 0, 1, &renderResources.vertexBuffer, offsets);
	vkCmdBindDescriptorSets(
		g_commandBuffers[g_presentationInfo.nextImageIndex],
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		g_pipelineLayout,
		0,
		1, &renderResources.descriptorSet,
		0, NULL);
	vkCmdDraw(g_commandBuffers[g_presentationInfo.nextImageIndex], sizeof(Quad) / 7, 1, 0, 0);
}

static VkInstance r_createInstance()
{
	VkResult result;

	uint32_t desiredInstanceExtensionCount = 2;
	const char *const desiredInstanceExtensions[] =
	{
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME
	};

	uint32_t instanceExtensionsCount;
	result = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionsCount, NULL);
	assert(result == VK_SUCCESS);

	VkExtensionProperties *availableInstanceExtensions =
		(VkExtensionProperties*)malloc(instanceExtensionsCount * sizeof(VkExtensionProperties)); // TODO: free
	result = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionsCount, availableInstanceExtensions);
	assert(result == VK_SUCCESS);

	for (uint32_t i = 0; i < desiredInstanceExtensionCount; i++)
	{
		assert(r_isExtensionAvailable(desiredInstanceExtensions[i], instanceExtensionsCount, availableInstanceExtensions));
	}

	VkApplicationInfo applicationInfo = { 0 };
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pNext = NULL;
	applicationInfo.pApplicationName = "Vulkan";
	applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.pEngineName = "vulkan";
	applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

	VkInstanceCreateInfo instanceCreateInfo = { 0 };
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = NULL;
	instanceCreateInfo.flags = 0;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;
	instanceCreateInfo.enabledExtensionCount = desiredInstanceExtensionCount;
	instanceCreateInfo.ppEnabledExtensionNames = desiredInstanceExtensions;

	VkInstance instance;
	result = vkCreateInstance(&instanceCreateInfo, NULL, &instance);
	assert(result == VK_SUCCESS);

	free(availableInstanceExtensions);

	return instance;
}

// TODO: validation
static VkPhysicalDevice r_findFirstCompatibleDevice(
	VkInstance instance)
{
	VkResult result;

	uint32_t desiredExtensionsCount = 1;
	const char *const desiredExtensions[1] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	uint32_t physicalDeviceCount;
	result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, NULL);
	assert(result == VK_SUCCESS);

	VkPhysicalDevice *availablePhysicalDevices =
		(VkPhysicalDevice*)malloc(physicalDeviceCount * sizeof(VkPhysicalDevice));
	result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, availablePhysicalDevices);

	VkBool32 allExtensionsAvailable;
	for (uint32_t i = 0; i < physicalDeviceCount; i++)
	{
		allExtensionsAvailable = VK_FALSE;

		uint32_t extensionsCount;
		result = vkEnumerateDeviceExtensionProperties(availablePhysicalDevices[i], NULL, &extensionsCount, NULL);
		assert(result == VK_SUCCESS);

		VkExtensionProperties *extensions =
			(VkExtensionProperties*)malloc(extensionsCount * sizeof(VkExtensionProperties)); // TODO: free
		result = vkEnumerateDeviceExtensionProperties(availablePhysicalDevices[i], NULL,
			&extensionsCount, extensions);
		assert(result == VK_SUCCESS);

		for (uint32_t j = 0; j < desiredExtensionsCount; j++)
		{
			if (r_isExtensionAvailable(desiredExtensions[j], extensionsCount, extensions))
			{
				allExtensionsAvailable = VK_TRUE;
			}
			else
			{
				allExtensionsAvailable = VK_FALSE;
				break;
			}
		}
		free(extensions);
		if (allExtensionsAvailable == VK_TRUE)
		{
			return availablePhysicalDevices[i];
		}
	}
	assert(allExtensionsAvailable != VK_FALSE);
	return NULL;
}

static VkDevice r_createLogicalDevice(
	VkPhysicalDevice physicalDevice,
	uint32_t queueFamiliesCount,
	VkQueueFamilyProperties *queueFamiliesProperties)
{
	VkResult result;
	//uint32_t deviceQueueCreateInfoCount;
	//VkDeviceQueueCreateInfo *deviceQueueCreateInfos;

	uint32_t desiredDeviceExtensionCount = 1;
	const char *const desiredDeviceExtensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	// TODO: this could be more robust
	float priority = 1.0f;

	int32_t graphicsQueueFamilyIndex = r_findQueueFamilyIndex(queueFamiliesCount, queueFamiliesProperties, VK_QUEUE_GRAPHICS_BIT);

	VkDeviceQueueCreateInfo queueCreateInfo = { 0 };
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.pNext = NULL;
	queueCreateInfo.flags = 0;
	queueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = &priority;

	VkDeviceCreateInfo deviceCreateInfo = { 0 };
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = NULL;
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
	deviceCreateInfo.enabledLayerCount = 0; // deprecated 
	deviceCreateInfo.ppEnabledLayerNames = NULL; // deprecated 
	deviceCreateInfo.enabledExtensionCount = desiredDeviceExtensionCount;
	deviceCreateInfo.ppEnabledExtensionNames = desiredDeviceExtensions;
	deviceCreateInfo.pEnabledFeatures = NULL;

	VkDevice device;
	result = vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, &device);
	assert(result == VK_SUCCESS);

	return device;
}

static VkSurfaceKHR r_createPresentationSurface(
	VkInstance instance,
	HINSTANCE hInstance,
	HWND window)
{
	VkResult result;

	VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfo = { 0 };
	win32SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	win32SurfaceCreateInfo.pNext = NULL;
	win32SurfaceCreateInfo.flags = 0;
	win32SurfaceCreateInfo.hinstance = hInstance;
	win32SurfaceCreateInfo.hwnd = window;

	VkSurfaceKHR presentationSurface;
	result = vkCreateWin32SurfaceKHR(instance, &win32SurfaceCreateInfo, NULL, &presentationSurface);
	assert(result == VK_SUCCESS);

	return presentationSurface;
}

static VkSwapchainKHR r_createSwapChain(
	VkDevice device,
	VkSurfaceKHR presentationSurface,
	uint32_t minImageCount,
	VkSurfaceFormatKHR surfaceFormat,
	VkExtent2D extent,
	VkPresentModeKHR presentMode,
	VkSwapchainKHR oldSwapchain)
{
	VkResult result;

	VkSwapchainCreateInfoKHR swapchainCreateInfo = { 0 };
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.pNext = NULL;
	swapchainCreateInfo.flags = 0;
	swapchainCreateInfo.surface = presentationSurface;
	swapchainCreateInfo.minImageCount = minImageCount;
	swapchainCreateInfo.imageFormat = surfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainCreateInfo.imageExtent = extent;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.queueFamilyIndexCount = 0;
	swapchainCreateInfo.pQueueFamilyIndices = NULL;
	swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = presentMode;
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = oldSwapchain;

	VkSwapchainKHR swapchain;
	result = vkCreateSwapchainKHR(device, &swapchainCreateInfo, NULL, &swapchain);
	assert(result == VK_SUCCESS);

	return swapchain;
}

static VkCommandPool r_createCommandPool(
	VkDevice device,
	VkCommandPoolCreateFlags createFlags,
	uint32_t queueFamilyIndex)
{
	VkResult result;

	VkCommandPoolCreateInfo commandPoolCreateInfo = { 0 };
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.pNext = NULL;
	commandPoolCreateInfo.flags = createFlags;
	commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;

	VkCommandPool commandPool;
	result = vkCreateCommandPool(device, &commandPoolCreateInfo, NULL, &commandPool);
	assert(result == VK_SUCCESS);

	return commandPool;
}

static VkCommandBuffer *r_allocateCommandBuffers(
	VkDevice device,
	VkCommandPool commandPool,
	VkCommandBufferLevel commandBufferLevel,
	uint32_t commandBufferCount)
{
	VkResult result;

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = { 0 };
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = NULL;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = commandBufferLevel;
	commandBufferAllocateInfo.commandBufferCount = commandBufferCount;

	VkCommandBuffer *commandBuffers = (VkCommandBuffer*)malloc(commandBufferCount * sizeof(VkCommandBuffer));
	result = vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, commandBuffers);
	assert(result == VK_SUCCESS);

	return commandBuffers;
}

static VkShaderModule r_createShaderModule(
	VkDevice device,
	size_t codeSize,
	const uint32_t *code)
{
	VkResult result;

	VkShaderModuleCreateInfo shaderModuleCreateInfo = { 0 };
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.pNext = NULL;
	shaderModuleCreateInfo.flags = 0;
	shaderModuleCreateInfo.codeSize = codeSize;
	shaderModuleCreateInfo.pCode = code;

	VkShaderModule shaderModule;
	result = vkCreateShaderModule(device, &shaderModuleCreateInfo, NULL, &shaderModule);
	assert(result == VK_SUCCESS);

	return shaderModule;
}

static VkRenderPass r_createRenderPass(
	VkDevice device,
	VkFormat format)
{
	VkResult result;

	VkAttachmentDescription colorAttachmentDescription = { 0 };
	colorAttachmentDescription.flags = 0;
	colorAttachmentDescription.format = format;
	colorAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentReference = { 0 };
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription = { 0 };
	subpassDescription.flags = 0;
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pInputAttachments = NULL;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorAttachmentReference;
	subpassDescription.pResolveAttachments = NULL;
	subpassDescription.pDepthStencilAttachment = NULL;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = NULL;

	VkSubpassDependency subpassDependency = { 0 };
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.dstSubpass = 0;
	subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.srcAccessMask = 0;
	subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependency.dependencyFlags = 0;

	VkRenderPassCreateInfo renderPassCreateInfo = { 0 };
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.pNext = NULL;
	renderPassCreateInfo.flags = 0;
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = &colorAttachmentDescription;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassDescription;
	renderPassCreateInfo.dependencyCount = 1;
	renderPassCreateInfo.pDependencies = &subpassDependency;

	VkRenderPass renderPass;
	result = vkCreateRenderPass(device, &renderPassCreateInfo, NULL, &renderPass);
	assert(result == VK_SUCCESS);

	return renderPass;
}

static VkPipelineLayout r_createPipelineLayout(
	VkDevice device,
	uint32_t descriptorSetLayoutCount,
	VkDescriptorSetLayout *descriptorSetLayouts,
	uint32_t pushConstantRangeCount,
	VkPushConstantRange *pushConstantRanges)
{
	VkResult result;

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = { 0 };
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pNext = NULL;
	pipelineLayoutCreateInfo.flags = 0;
	pipelineLayoutCreateInfo.setLayoutCount = descriptorSetLayoutCount;
	pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts;
	pipelineLayoutCreateInfo.pushConstantRangeCount = pushConstantRangeCount;
	pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRanges;

	VkPipelineLayout pipelineLayout;
	result = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, NULL, &pipelineLayout);
	assert(result == VK_SUCCESS);

	return pipelineLayout;
}

static VkPipeline r_createGraphicsPipeline(
	VkDevice device,
	uint32_t shaderStageCount,
	VkShaderStageFlags *shaderStages,
	VkShaderModule *shaderModules,
	uint32_t vertexBindingDescriptionCount,
	VkVertexInputBindingDescription *vertexBindingDescriptions,
	uint32_t vertexAttributeDescriptionCount,
	VkVertexInputAttributeDescription *vertexAttributeDescriptions,
	uint32_t viewportCount,
	VkViewport *viewports,
	uint32_t scissorCount,
	VkRect2D *scissors,
	VkPipelineLayout pipelineLayout,
	VkRenderPass renderPass)
{
	VkResult result;

	VkPipelineShaderStageCreateInfo *shaderStageCreateInfos =
		(VkPipelineShaderStageCreateInfo*)malloc(shaderStageCount * sizeof(VkPipelineShaderStageCreateInfo)); // TODO: free
	for (uint32_t i = 0; i < shaderStageCount; i++)
	{
		shaderStageCreateInfos[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageCreateInfos[i].pNext = NULL;
		shaderStageCreateInfos[i].flags = 0;
		shaderStageCreateInfos[i].stage = shaderStages[i];
		shaderStageCreateInfos[i].module = shaderModules[i];
		shaderStageCreateInfos[i].pName = "main";
		shaderStageCreateInfos[i].pSpecializationInfo = NULL;
	}

	VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = { 0 };
	vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputStateCreateInfo.pNext = NULL;
	vertexInputStateCreateInfo.flags = 0;
	vertexInputStateCreateInfo.vertexBindingDescriptionCount = vertexBindingDescriptionCount;
	vertexInputStateCreateInfo.pVertexBindingDescriptions = vertexBindingDescriptions;
	vertexInputStateCreateInfo.vertexAttributeDescriptionCount = vertexAttributeDescriptionCount;
	vertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions;

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = { 0 };
	inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyStateCreateInfo.pNext = NULL;
	inputAssemblyStateCreateInfo.flags = 0;
	inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo = { 0 };
	viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.pNext = NULL;
	viewportStateCreateInfo.flags = 0;
	viewportStateCreateInfo.viewportCount = viewportCount;
	viewportStateCreateInfo.pViewports = viewports;
	viewportStateCreateInfo.scissorCount = scissorCount;
	viewportStateCreateInfo.pScissors = scissors;

	VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = { 0 };
	rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationStateCreateInfo.pNext = NULL;
	rasterizationStateCreateInfo.flags = 0;
	rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
	rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
	rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
	rasterizationStateCreateInfo.depthBiasClamp = 0.0f;
	rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;
	rasterizationStateCreateInfo.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = { 0 };
	multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleStateCreateInfo.pNext = NULL;
	multisampleStateCreateInfo.flags = 0;
	multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
	multisampleStateCreateInfo.minSampleShading = 1.0f;
	multisampleStateCreateInfo.pSampleMask = NULL;
	multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
	multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachmentState = { 0 };
	colorBlendAttachmentState.blendEnable = VK_TRUE;
	colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachmentState.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = { 0 };
	colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendStateCreateInfo.pNext = NULL;
	colorBlendStateCreateInfo.flags = 0;
	colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
	colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
	colorBlendStateCreateInfo.attachmentCount = 1;
	colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;
	colorBlendStateCreateInfo.blendConstants[0] = 0.0f;
	colorBlendStateCreateInfo.blendConstants[1] = 0.0f;
	colorBlendStateCreateInfo.blendConstants[2] = 0.0f;
	colorBlendStateCreateInfo.blendConstants[3] = 0.0f;

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = { 0 };
	graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineCreateInfo.pNext = NULL;
	graphicsPipelineCreateInfo.flags = 0;
	graphicsPipelineCreateInfo.stageCount = shaderStageCount;
	graphicsPipelineCreateInfo.pStages = shaderStageCreateInfos;
	graphicsPipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
	graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
	graphicsPipelineCreateInfo.pTessellationState = NULL;
	graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
	graphicsPipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
	graphicsPipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
	graphicsPipelineCreateInfo.pDepthStencilState = NULL;
	graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
	graphicsPipelineCreateInfo.pDynamicState = NULL;
	graphicsPipelineCreateInfo.layout = pipelineLayout;
	graphicsPipelineCreateInfo.renderPass = renderPass;
	graphicsPipelineCreateInfo.subpass = 0;
	graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	graphicsPipelineCreateInfo.basePipelineIndex = -1;

	VkPipeline pipeline;
	result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, NULL, &pipeline);
	assert(result == VK_SUCCESS);

	return pipeline;
}

static VkFramebuffer r_createFramebuffer(
	VkDevice device,
	VkRenderPass renderPass,
	uint32_t imageViewCount,
	VkImageView imageView,
	VkExtent2D extent)
{
	VkResult result;

	VkFramebufferCreateInfo framebufferCreateInfo = { 0 };
	framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferCreateInfo.pNext = NULL;
	framebufferCreateInfo.flags = 0;
	framebufferCreateInfo.renderPass = renderPass;
	framebufferCreateInfo.attachmentCount = 1;
	framebufferCreateInfo.pAttachments = &imageView;
	framebufferCreateInfo.width = extent.width;
	framebufferCreateInfo.height = extent.height;
	framebufferCreateInfo.layers = 1;

	VkFramebuffer framebuffer;
	result = vkCreateFramebuffer(device, &framebufferCreateInfo, NULL, &framebuffer);
	assert(result == VK_SUCCESS);

	return framebuffer;
}

static VkBuffer r_createBuffer(
	VkDevice device,
	VkDeviceSize bufferSize,
	VkBufferUsageFlags usageFlags)
{
	VkResult result;

	VkBufferCreateInfo bufferCreateInfo = { 0 };
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = NULL;
	bufferCreateInfo.flags = 0;
	bufferCreateInfo.size = bufferSize;
	bufferCreateInfo.usage = usageFlags;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkBuffer buffer;
	result = vkCreateBuffer(device, &bufferCreateInfo, NULL, &buffer);
	assert(result == VK_SUCCESS);

	return buffer;
}

static VkDeviceMemory r_allocateBufferMemory(
	VkDevice device,
	VkBuffer buffer,
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties,
	VkMemoryPropertyFlags memoryPropertyFlags)
{
	VkResult result;

	VkMemoryRequirements bufferMemoryRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &bufferMemoryRequirements);

	int32_t memoryTypeIndex = r_findCapableMemoryIndex(
		bufferMemoryRequirements,
		physicalDeviceMemoryProperties,
		memoryPropertyFlags);
	assert(memoryTypeIndex >= 0);

	VkMemoryAllocateInfo bufferMemoryAllocateInfo = { 0 };
	bufferMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	bufferMemoryAllocateInfo.pNext = NULL;
	bufferMemoryAllocateInfo.allocationSize = bufferMemoryRequirements.size;
	bufferMemoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

	VkDeviceMemory bufferMemory;
	result = vkAllocateMemory(device, &bufferMemoryAllocateInfo, NULL, &bufferMemory);
	assert(result == VK_SUCCESS);

	return bufferMemory;
}

static void r_copyDataToBuffer(
	VkDevice device,
	size_t dataSize,
	void *data,
	VkDeviceMemory bufferMemory,
	VkDeviceSize offset)
{
	VkResult result;

	void *dst;
	result = vkMapMemory(device, bufferMemory, offset, VK_WHOLE_SIZE, 0, &dst);
	assert(result == VK_SUCCESS);
	memcpy(dst, data, dataSize);
	vkUnmapMemory(device, bufferMemory);
}

static void r_copyBufferToImage(
	uint32_t width,
	uint32_t height,
	VkBuffer buffer,
	VkImage image)
{
	VkBufferImageCopy bufferImageCopy = { 0 };
	bufferImageCopy.bufferOffset = 0;
	bufferImageCopy.bufferRowLength = 0;
	bufferImageCopy.bufferImageHeight = 0;
	bufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	bufferImageCopy.imageSubresource.mipLevel = 0;
	bufferImageCopy.imageSubresource.baseArrayLayer = 0;
	bufferImageCopy.imageSubresource.layerCount = 1;
	bufferImageCopy.imageOffset.x = 0;
	bufferImageCopy.imageOffset.y = 0;
	bufferImageCopy.imageOffset.z = 0;
	bufferImageCopy.imageExtent.width = width;
	bufferImageCopy.imageExtent.height = height;
	bufferImageCopy.imageExtent.depth = 1;

	VkCommandBuffer commandBuffer = r_beginOneShotRecording(g_device, g_commandPool);

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);

	r_endOneShotRecording(g_device, g_commandPool, commandBuffer);
}

static VkImageView r_createImageView(
	VkDevice device,
	VkImage image,
	VkImageViewType viewType,
	VkFormat format)
{
	VkResult result;

	VkImageViewCreateInfo imageViewCreateInfo = { 0 };
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.pNext = NULL;
	imageViewCreateInfo.flags = 0;
	imageViewCreateInfo.image = image;
	imageViewCreateInfo.viewType = viewType;
	imageViewCreateInfo.format = format;
	imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	result = vkCreateImageView(device, &imageViewCreateInfo, NULL, &imageView);
	assert(result == VK_SUCCESS);

	return imageView;
}

static VkDeviceMemory r_allocateImageMemory(
	VkDevice device,
	VkImage image,
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties,
	VkMemoryPropertyFlags memoryPropertyFlags)
{
	VkResult result;

	VkMemoryRequirements imageMemoryRequirements;
	vkGetImageMemoryRequirements(device, image, &imageMemoryRequirements);

	int32_t memoryTypeIndex = r_findCapableMemoryIndex(
		imageMemoryRequirements,
		physicalDeviceMemoryProperties,
		memoryPropertyFlags);
	assert(memoryTypeIndex >= 0);

	VkMemoryAllocateInfo imageMemoryAllocateInfo = { 0 };
	imageMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	imageMemoryAllocateInfo.pNext = NULL;
	imageMemoryAllocateInfo.allocationSize = imageMemoryRequirements.size;
	imageMemoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

	VkDeviceMemory imageMemory;
	result = vkAllocateMemory(device, &imageMemoryAllocateInfo, NULL, &imageMemory);
	assert(result == VK_SUCCESS);

	return imageMemory;
}
static VkSemaphore r_createSemaphore(VkDevice device)
{
	VkResult result;

	VkSemaphoreCreateInfo semaphoreCreateInfo = { 0 };
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = NULL;
	semaphoreCreateInfo.flags = 0;

	VkSemaphore semaphore;
	result = vkCreateSemaphore(device, &semaphoreCreateInfo, NULL, &semaphore);
	assert(result == VK_SUCCESS);

	return semaphore;
}

static VkDescriptorSetLayout r_createDescriptorSetLayout(
	VkDevice device,
	uint32_t bindingCount,
	VkDescriptorSetLayoutBinding  *descriptorSetLayoutBindings)
{
	VkResult result;

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = { 0 };
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.pNext = NULL;
	descriptorSetLayoutCreateInfo.flags = 0;
	descriptorSetLayoutCreateInfo.bindingCount = bindingCount;
	descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings;

	VkDescriptorSetLayout descriptorSetLayout;
	result = vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, NULL, &descriptorSetLayout);
	assert(result == VK_SUCCESS);

	return descriptorSetLayout;
}

static int32_t r_findQueueFamilyIndex(
	uint32_t queueFamilyCount,
	VkQueueFamilyProperties *queueFamilyProperties,
	VkQueueFlags desiredCapabilites)
{
	int32_t queueIndex = -1;
	for (uint32_t i = 0; i < queueFamilyCount; i++)
	{
		if (queueFamilyProperties[i].queueCount > 0 &&
			(queueFamilyProperties[i].queueFlags & desiredCapabilites) != 0)
		{
			queueIndex = (int32_t)i;
			break;
		}
	}
	return queueIndex;
}

static int32_t r_findCapableMemoryIndex(
	VkMemoryRequirements memoryRequirements,
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties,
	VkMemoryPropertyFlags memoryPropertyFlags)
{
	int32_t memoryPropertyIndex = -1;
	for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((memoryRequirements.memoryTypeBits & (1 << i)) && // the fuck?
			(physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags)
			== memoryPropertyFlags)
		{
			memoryPropertyIndex = (int32_t)i;
			break;
		}
	}
	return memoryPropertyIndex;
}

static boolean r_isExtensionAvailable(
	const char* const desiredExtension,
	uint32_t availableExtensionsCount,
	VkExtensionProperties *availableExtensions)
{
	for (uint32_t i = 0; i < availableExtensionsCount; i++)
	{
		if (strcmp(desiredExtension, availableExtensions[i].extensionName) == 0)
			return TRUE;
	}
	return FALSE;
}

// OUT: Most desired and available present mode, or VK_PRESENT_MODE_FIFO_KHR
static VkPresentModeKHR r_getBestMatchPresentMode(
	VkPhysicalDevice physicalDevice,
	VkSurfaceKHR presentationSurface,
	uint32_t desiredPresentModesCount,
	VkPresentModeKHR *desiredPresentModes) // List of desired presentmodes orded by priority 
{
	VkResult result;

	uint32_t availablePresentModesCount;
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, presentationSurface, &availablePresentModesCount, NULL);
	assert(result == VK_SUCCESS);

	VkPresentModeKHR *availablePresentModes =
		(VkPresentModeKHR*)malloc(availablePresentModesCount * sizeof(VkPresentModeKHR));
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, presentationSurface, &availablePresentModesCount, availablePresentModes);
	assert(result == VK_SUCCESS);

	for (uint32_t i = 0; i < desiredPresentModesCount; i++)
	{
		if (r_isPresentModeAvailable(desiredPresentModes[i], availablePresentModesCount, availablePresentModes))
		{
			free(availablePresentModes);
			return desiredPresentModes[i];
		}
	}
	free(availablePresentModes);
	return VK_PRESENT_MODE_FIFO_KHR;
}

static boolean r_isPresentModeAvailable(
	VkPresentModeKHR desiredPresentMode,
	uint32_t availablePresentModesCount,
	VkPresentModeKHR *availablePresentModes)
{
	for (uint32_t i = 0; i < availablePresentModesCount; i++)
	{
		if (desiredPresentMode == availablePresentModes[i])
			return TRUE;
	}
	return FALSE;
}

static VkSurfaceFormatKHR r_getBestMatchSurfaceFormat(
	VkPhysicalDevice physicalDevice,
	VkSurfaceKHR presentationSurface,
	uint32_t desiredSurfaceFormatsCount,
	VkSurfaceFormatKHR *desiredSurfaceFormats)
{
	VkResult result;

	VkSurfaceFormatKHR defaultSurfaceFormat = {
		.format = VK_FORMAT_B8G8R8A8_UNORM,
		.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
	};

	uint32_t availableSurfaceFormatsCount;
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, presentationSurface, &availableSurfaceFormatsCount, NULL);
	assert(result == VK_SUCCESS);

	VkSurfaceFormatKHR *availableSurfaceFormats =
		(VkSurfaceFormatKHR*)malloc(availableSurfaceFormatsCount * sizeof(VkSurfaceFormatKHR));
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, presentationSurface, &availableSurfaceFormatsCount, availableSurfaceFormats);
	assert(result == VK_SUCCESS);

	for (uint32_t i = 0; i < desiredSurfaceFormatsCount; i++)
	{
		if (r_isSurfaceFormatAvailable(desiredSurfaceFormats[i], availableSurfaceFormatsCount, availableSurfaceFormats))
		{
			free(availableSurfaceFormats);
			return desiredSurfaceFormats[i];
		}
	}
	free(availableSurfaceFormats);

	return defaultSurfaceFormat;
}

static boolean r_isSurfaceFormatAvailable(
	VkSurfaceFormatKHR desiredSurfaceFormat,
	uint32_t availableSurfaceFormatsCount,
	VkSurfaceFormatKHR *availableSurfaceFormats
)
{
	for (uint32_t i = 0; i < availableSurfaceFormatsCount; i++)
	{
		if (desiredSurfaceFormat.colorSpace == availableSurfaceFormats[i].colorSpace &&
			desiredSurfaceFormat.format == availableSurfaceFormats[i].format)
		{
			return TRUE;
		}
	}
	return FALSE;
}

static Quad r_createQuad(float width, float height)
{
	Quad quad =
	{
		{
			{ { -width / 2.0f,  height / 2.0f }, { 0.0f, 0.0f, 1.0f}, { 0.0f, 0.0f } }, // top left
			{ {  width / 2.0f, -height / 2.0f }, { 1.0f, 1.0f, 1.0f}, { 1.0f, 1.0f } }, // bottom right
			{ { -width / 2.0f, -height / 2.0f }, { 0.0f, 0.0f, 1.0f}, { 0.0f, 1.0f } }, // bottom left
			{ { -width / 2.0f,  height / 2.0f }, { 0.0f, 0.0f, 1.0f}, { 0.0f, 0.0f } }, // top left
			{ {  width / 2.0f,  height / 2.0f }, { 1.0f, 1.0f, 1.0f}, { 1.0f, 0.0f } }, // top right
			{ {  width / 2.0f, -height / 2.0f }, { 1.0f, 1.0f, 1.0f}, { 1.0f, 1.0f } }  // bottom right
		}
	};
	return quad;
}

static void r_transitionTextureImageLayout(
	VkImage image,
	VkAccessFlags oldAccessMask,
	VkAccessFlags newAccessMask,
	VkImageLayout oldLayout,
	VkImageLayout newLayout)
{
	r_transitionImageLayout(
		oldAccessMask, newAccessMask,
		oldLayout, newLayout,
		0, 0,
		image,
		VK_IMAGE_ASPECT_COLOR_BIT);
}

static void r_transitionImageLayout(
	VkAccessFlags sourceAccessMask,
	VkAccessFlags desinationAccessMask,
	VkImageLayout oldLayout,
	VkImageLayout newLayout,
	uint32_t sourceQueueFamilyIndex,
	uint32_t destinationQueueFamilyIndex,
	VkImage image,
	VkImageAspectFlags imageAspectMask)
{
	VkCommandBuffer commandBuffer = r_beginOneShotRecording(g_device, g_commandPool);

	VkImageMemoryBarrier imageMemoryBarrier = { 0 };
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.pNext = NULL;
	imageMemoryBarrier.srcAccessMask = sourceAccessMask;
	imageMemoryBarrier.dstAccessMask = desinationAccessMask;
	imageMemoryBarrier.oldLayout = oldLayout;
	imageMemoryBarrier.newLayout = newLayout;
	imageMemoryBarrier.srcQueueFamilyIndex = sourceQueueFamilyIndex;
	imageMemoryBarrier.dstQueueFamilyIndex = destinationQueueFamilyIndex;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange.aspectMask = imageAspectMask;
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarrier.subresourceRange.levelCount = 1;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarrier.subresourceRange.layerCount = 1;

	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		0,
		0, NULL,
		0, NULL,
		1, &imageMemoryBarrier);

	r_endOneShotRecording(g_device, g_commandPool, commandBuffer);
}

static VkCommandBuffer r_beginOneShotRecording(
	VkDevice device,
	VkCommandPool commandPool)
{
	VkResult result;

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = { 0 };
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = NULL;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	result = vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer);
	assert(result == VK_SUCCESS);

	VkCommandBufferBeginInfo commandBufferBeginInfo = { 0 };
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = NULL;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	commandBufferBeginInfo.pInheritanceInfo = NULL;

	result = vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
	assert(result == VK_SUCCESS);

	return commandBuffer;
}

static void r_endOneShotRecording(
	VkDevice device,
	VkCommandPool commandPool,
	VkCommandBuffer commandBuffer)
{
	VkResult result;
	result = vkEndCommandBuffer(commandBuffer);
	assert(result == VK_SUCCESS);

	VkSubmitInfo submitInfo = { 0 };
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = NULL;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = NULL;
	submitInfo.pWaitDstStageMask = NULL;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = NULL;

	vkQueueSubmit(g_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(g_graphicsQueue);

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

static void r_createQuadVertexBufferAndMemory(
	Quad quad,
	VkBuffer* buffer,
	VkDeviceMemory *memory)
{
	VkResult result;

	*buffer = r_createBuffer(g_device, sizeof(Quad), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	*memory = r_allocateBufferMemory(
		g_device,
		*buffer,
		g_physicalDeviceMemoryProperties,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	result = vkBindBufferMemory(g_device, *buffer, *memory, 0);
	assert(result == VK_SUCCESS);
	r_copyDataToBuffer(g_device, sizeof(Quad), &quad, *memory, 0);
}

static uint32_t r_getSecondaryCommandBufferIndex()
{
	assert(g_commandBuffersSecondaryActive < g_commandBuffersSecondaryCount);
	return g_commandBuffersSecondaryActive++;
}

Texture r_createTexture(char * filename)
{
	VkResult result;
	Texture texture = { 0 };

	uint32_t componentsCount = 0;
	unsigned char *stbiData = stbi_load(filename, &texture.width, &texture.height, &componentsCount, STBI_rgb_alpha);
	assert(componentsCount > 0);
	size_t dataSize = texture.width * texture.height * 4;

	VkDeviceSize imageSize = dataSize;

	VkBuffer stagingBuffer = r_createBuffer(g_device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	VkDeviceMemory stagingBufferMemory = r_allocateBufferMemory(
		g_device,
		stagingBuffer,
		g_physicalDeviceMemoryProperties,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	result = vkBindBufferMemory(g_device, stagingBuffer, stagingBufferMemory, 0);
	assert(result == VK_SUCCESS);
	r_copyDataToBuffer(g_device, imageSize, stbiData, stagingBufferMemory, 0);
	stbi_image_free(stbiData);

	VkImageCreateInfo textureImageCreateInfo = { 0 };
	textureImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	textureImageCreateInfo.pNext = NULL;
	textureImageCreateInfo.flags = 0;
	textureImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	textureImageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	textureImageCreateInfo.extent.width = (uint32_t)texture.width;
	textureImageCreateInfo.extent.height = (uint32_t)texture.height;
	textureImageCreateInfo.extent.depth = 1;
	textureImageCreateInfo.mipLevels = 1;
	textureImageCreateInfo.arrayLayers = 1;
	textureImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	textureImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	textureImageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	textureImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	textureImageCreateInfo.queueFamilyIndexCount = 0;
	textureImageCreateInfo.pQueueFamilyIndices = NULL;
	textureImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

	result = vkCreateImage(g_device, &textureImageCreateInfo, NULL, &texture.image);
	assert(result == VK_SUCCESS);

	texture.imageMemory = r_allocateImageMemory(
		g_device, texture.image,
		g_physicalDeviceMemoryProperties,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	result = vkBindImageMemory(g_device, texture.image, texture.imageMemory, 0);
	assert(result == VK_SUCCESS);

	r_transitionTextureImageLayout(
		texture.image,
		VK_ACCESS_HOST_WRITE_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	r_copyBufferToImage(texture.width, texture.height, stagingBuffer, texture.image);
	r_transitionTextureImageLayout(
		texture.image,
		VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	texture.imageView = r_createImageView(
		g_device,
		texture.image,
		VK_IMAGE_VIEW_TYPE_2D,
		VK_FORMAT_R8G8B8A8_UNORM);

	VkSamplerCreateInfo samplerCreateInfo = { 0 };
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.pNext = NULL;
	samplerCreateInfo.flags = 0;
	samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
	samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.anisotropyEnable = VK_FALSE;
	samplerCreateInfo.maxAnisotropy = 1;
	samplerCreateInfo.compareEnable = VK_FALSE;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = 0.0f;
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;


	result = vkCreateSampler(g_device, &samplerCreateInfo, NULL, &texture.sampler);
	assert(result == VK_SUCCESS);

	return texture;
}

LRESULT CALLBACK WndProc(
	HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		exit(0);
		return 0;
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

static HWND r_createAndRegisterWindow(HINSTANCE hInstance)
{
	WNDCLASSEX windowClass = { 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WndProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = hInstance;
	windowClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = L"windowClass";
	windowClass.hIconSm = LoadIcon(windowClass.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

	RegisterClassEx(&windowClass);

	RECT clientArea = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	AdjustWindowRect(&clientArea, WS_OVERLAPPEDWINDOW, FALSE);

	// TODO: clean up
	HWND window = CreateWindow(
		L"windowClass", // class name
		L"vulkan", // window name
		WS_OVERLAPPEDWINDOW, // TODO: consider flags
		CW_USEDEFAULT, // pos x
		CW_USEDEFAULT, // pos y
		clientArea.right - clientArea.left, // width
		clientArea.bottom - clientArea.top, // height
		NULL,
		NULL,
		hInstance,
		NULL
	);

	return window;
}

static ShaderInfo r_readShaderFile(const char *filename)
{
	FILE *shaderFile;
	assert(fopen_s(&shaderFile, filename, "rb") != EINVAL);

	fseek(shaderFile, 0, SEEK_END);
	size_t shaderFileSize = ftell(shaderFile);
	fseek(shaderFile, 0, SEEK_SET);

	ShaderInfo shader = { 0 };

	shader.codeSize = shaderFileSize;
	shader.shaderCode = (char*)malloc(shaderFileSize + 1);

	fread_s(shader.shaderCode, shaderFileSize + 1, 1, shaderFileSize, shaderFile);

	fclose(shaderFile);

	return shader;
}