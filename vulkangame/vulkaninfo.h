#pragma once
#include "vulkan\vulkan.h"

// TODO: padding

typedef struct AppInfo
{
	VkInstance instance;
	uint32_t extensionsCount;
	VkExtensionProperties *extensionProperties;
} AppInfo;

typedef struct GpuInfo
{
	VkPhysicalDevice device;
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceFeatures features;
	VkPhysicalDeviceMemoryProperties memoryProperties;
	uint32_t extensionsCount;
	VkExtensionProperties *extensionProperties;
	uint32_t queueFamiliesCount;
	VkQueueFamilyProperties *queueFamilyProperties;
} GpuInfo;

typedef struct DeviceInfo
{
	VkDevice device;
	uint32_t queueFamiliesCount;
	VkQueueFamilyProperties *queueFamiliesProperties;
	uint32_t presentQueueFamilyIndex;
	VkQueue presentQueue;
} DeviceInfo;

typedef struct SwapchainInfo
{
	VkSwapchainKHR swapchain;
	VkSwapchainKHR oldSwapchain;
	uint32_t imagesCount;
	uint32_t nextImageIndex;
	VkImage *images;
	VkMemoryRequirements *imageMemoryRequirements;
	VkDeviceMemory *imageMemory;
	VkImageView *imageViews;
	VkImageSubresourceRange imageSubresourceRange;
	VkFormat format;
	VkColorSpaceKHR colorSpace;
	VkFramebuffer *framebuffers;
} SwapchainInfo;

typedef struct CommandPoolInfo
{
	VkCommandPool commandPool;
	uint32_t commandBufferCount;
	VkCommandBuffer *commandBuffers;

} CommandPoolInfo;

typedef struct PresentInfo
{
	HWND window;
	VkSurfaceKHR surface;
	uint32_t presentModesCount;
	VkPresentModeKHR *presentModes;
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	uint32_t imagesCount;
	VkExtent2D imagesExtent;
	uint32_t surfaceFormatsCount;
	VkSurfaceFormatKHR *surfaceFormats;
	VkFence fence;
	VkSemaphore semaphore;
	VkClearColorValue clearColor;
} PresentInfo;

typedef struct ImageTransitions
{
	VkImageMemoryBarrier currentLayout;
	VkImageMemoryBarrier undefinedToTransfer;
	VkImageMemoryBarrier transferToPresent;
} ImageTransitions;

typedef struct Shaders
{
	char *vertexShader;
	size_t vertexShaderFileSize;
	char *fragmentShader;
	size_t fragmentShaderFileSize;
} Shaders;

typedef struct GraphicsPipeline
{
	VkPipeline pipeline;
	VkPipelineLayout layout;
	VkRenderPass renderPass;
	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;

} GraphicsPipeline;

typedef struct BufferInfo
{
	VkBuffer buffer;
	VkMemoryRequirements memoryRequirements;
	VkDeviceMemory memory;
} VertexBuffer, UniformBuffer;

typedef struct DescriptorInfo
{
	VkDescriptorSetLayout setLayout;
	VkDescriptorPool pool;
	VkDescriptorSet set;
} DescriptorInfo;

typedef struct OsParams
{
	HINSTANCE hInstance;
	HINSTANCE hPrevInstance;
	LPSTR lpCmdLine;
	int nShowCmd;
} OsParams;