#define VK_USE_PLATFORM_WIN32_KHR

#include <Windows.h>
#include <vulkan\vulkan.h>
#include <stdio.h>
#include <tchar.h>
#include <assert.h>
#include "renderer.h"
#include "vulkaninfo.h"

// File scope functions
static VkInstance r_createVkInstance();
static void r_createOSWindow();
static GpuInfo r_findFirstCompatibleGpu();
static void r_createLogicalDevice();
static void r_createPresentCommandPoolAndBuffers();
static void r_createPresentationSurface();
static void r_createPresentationSwapchain();
static void r_getPresentationSwapchainImages();
static void r_allocatePresentationSwapchainImageMemory();
static void r_createPresentationSwapchainImageViews();
static void r_createPresentationSynchronizationPrimitives();
static void r_initImageLayouts();
static void r_updateImageTransitionMemoryBarriers();
static void r_readShaders();
static void r_createDefaultShaderModules();
static void r_createRenderPass();
static void r_createDefaultGraphicsPipeline();
static void r_createFrameBuffers();
static void r_getAllGpus(
	GpuInfo *gpuInfos,
	VkPhysicalDevice *availableDevices,
	uint32_t gpuCount);
static int32_t r_getFirstCompatibleGpuIndex(GpuInfo *gpuInfos, uint32_t gpuCount);
static boolean r_isExtensionAvailable(
	const char *const desiredExtension,
	VkExtensionProperties *availableExtensions,
	uint32_t extensionsCount);
static HWND createAndRegisterWindow(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// File scope variables
static OsParams g_osParams = { 0 };
static VkInstance g_instance; // TODO: remove? i dunno
static AppInfo g_appInfo = { 0 };
static GpuInfo g_gpuInfo = { 0 };
static DeviceInfo g_deviceInfo = { 0 };
static CommandPoolInfo g_presentCommandPoolInfo = { 0 };
static PresentInfo g_presentInfo = { 0 };
static SwapchainInfo g_swapchainInfo = { 0 };
static ImageTransitions g_imageTransitions = { 0 };
static Shaders g_shaders = { 0 };
static GraphicsPipeline g_graphicsPipeline = { 0 };

void r_initVulkan(OsParams params)
{
	g_osParams = params;
	r_createOSWindow();

	g_instance = r_createVkInstance();
	g_gpuInfo = r_findFirstCompatibleGpu();
	r_createLogicalDevice();
	r_createPresentCommandPoolAndBuffers();
	r_createPresentationSurface();
	r_createPresentationSwapchain();
	r_getPresentationSwapchainImages();
	r_allocatePresentationSwapchainImageMemory();
	r_createPresentationSwapchainImageViews();
	r_createPresentationSynchronizationPrimitives();
	r_initImageLayouts();
	r_readShaders();
	r_createDefaultShaderModules();
	r_createRenderPass();
	r_createDefaultGraphicsPipeline();
	r_createFrameBuffers();
}

void r_destroyVulkan()
{
	// TODO: destroy
}

GpuInfo r_test_getGpuInfo()
{
	return g_gpuInfo;
}

DeviceInfo r_test_getDeviceInfo()
{
	return g_deviceInfo;
}

CommandPoolInfo r_test_getCommandPoolInfo()
{
	return g_presentCommandPoolInfo;
}

PresentInfo r_test_getPresentInfo()
{
	return g_presentInfo;
}

SwapchainInfo r_test_getSwapchainInfo()
{
	return g_swapchainInfo;
}

static void r_createOSWindow()
{
	g_presentInfo.window = createAndRegisterWindow(g_osParams.hInstance);
}

void r_showWindow()
{
	ShowWindow(g_presentInfo.window, g_osParams.nShowCmd);
}

// TODO: make sure extension is available
// TODO: move to struct
static VkInstance r_createVkInstance()
{
	VkResult result;

	const char *const desired_instance_extensions[2] = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME
	};

	uint32_t instance_extensions_count;
	result = vkEnumerateInstanceExtensionProperties(NULL, &instance_extensions_count, NULL);
	assert(result == VK_SUCCESS);

	VkExtensionProperties *available_instance_extensions =
		(VkExtensionProperties*)malloc(instance_extensions_count * sizeof(VkExtensionProperties)); // TODO: free
	result = vkEnumerateInstanceExtensionProperties(NULL, &instance_extensions_count, available_instance_extensions);
	assert(result == VK_SUCCESS);

	VkApplicationInfo application_info = { 0 };
	application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application_info.pNext = NULL;
	application_info.pApplicationName = "Vulkan";
	application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	application_info.pEngineName = "vulkan";
	application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	application_info.apiVersion = VK_MAKE_VERSION(1, 0, 0);

	VkInstanceCreateInfo instance_create_info = { 0 };
	instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_create_info.pNext = NULL;
	instance_create_info.flags = 0;
	instance_create_info.pApplicationInfo = &application_info;
	instance_create_info.enabledExtensionCount = 2;
	instance_create_info.ppEnabledExtensionNames = desired_instance_extensions;

	VkInstance instance;
	result = vkCreateInstance(&instance_create_info, NULL, &instance);
	assert(result == VK_SUCCESS);

	free(available_instance_extensions);

	return instance;
}

static GpuInfo r_findFirstCompatibleGpu()
{
	VkResult result;

	uint32_t gpuCount;
	result = vkEnumeratePhysicalDevices(g_instance, &gpuCount, NULL);
	assert(result == VK_SUCCESS);

	VkPhysicalDevice *availableDevices =
		(VkPhysicalDevice*)malloc(gpuCount * sizeof(VkPhysicalDevice));
	result = vkEnumeratePhysicalDevices(g_instance, &gpuCount, availableDevices);

	GpuInfo *gpuInfos =
		(GpuInfo*)malloc(gpuCount * sizeof(GpuInfo));

	r_getAllGpus(gpuInfos, availableDevices, gpuCount);

	int32_t gpuIndex = r_getFirstCompatibleGpuIndex(gpuInfos, gpuCount);
	assert(gpuIndex >= 0);

	GpuInfo gpu = { 0 };

	// TODO: test
	memcpy(&gpu, &gpuInfos[gpuIndex], sizeof(GpuInfo));
	gpu.extensionProperties = malloc(sizeof(VkExtensionProperties));
	gpu.queueFamilyProperties = malloc(sizeof(VkQueueFamilyProperties));
	memcpy(gpu.extensionProperties, gpuInfos[gpuIndex].extensionProperties, sizeof(VkExtensionProperties));
	memcpy(gpu.queueFamilyProperties, gpuInfos[gpuIndex].queueFamilyProperties, sizeof(VkQueueFamilyProperties));

	for (uint32_t i = 0; i < gpuCount; i++)
	{
		free(gpuInfos[i].extensionProperties);
		free(gpuInfos[i].queueFamilyProperties);
	}
	free(gpuInfos);
	free(availableDevices);

	return gpu;
}

static void r_getAllGpus(
	GpuInfo *gpuInfos,
	VkPhysicalDevice *availableDevices,
	uint32_t gpuCount)
{
	VkResult result;
	for (uint32_t i = 0; i < gpuCount; i++)
	{
		uint32_t extensionsCount;
		result = vkEnumerateDeviceExtensionProperties(availableDevices[i], NULL, &extensionsCount, NULL);
		assert(result == VK_SUCCESS);

		VkExtensionProperties *extensions =
			(VkExtensionProperties*)malloc(extensionsCount * sizeof(VkExtensionProperties)); // TODO: free
		result = vkEnumerateDeviceExtensionProperties(availableDevices[i], NULL,
			&extensionsCount, extensions);
		assert(result == VK_SUCCESS);

		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(availableDevices[i], &properties);

		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceFeatures(availableDevices[i], &features);

		uint32_t queueFamiliesCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(availableDevices[i], &queueFamiliesCount, NULL);
		assert(queueFamiliesCount > 0);

		VkQueueFamilyProperties *queueFamilyProperties =
			(VkQueueFamilyProperties*)malloc(queueFamiliesCount * sizeof(VkQueueFamilyProperties)); // TODO: free
		vkGetPhysicalDeviceQueueFamilyProperties(availableDevices[i], &queueFamiliesCount, queueFamilyProperties);

		VkPhysicalDeviceMemoryProperties memoryProperties;
		vkGetPhysicalDeviceMemoryProperties(availableDevices[i], &memoryProperties);

		gpuInfos[i].device = availableDevices[i];
		gpuInfos[i].properties = properties;
		gpuInfos[i].features = features;
		gpuInfos[i].memoryProperties = memoryProperties;
		gpuInfos[i].extensionsCount = extensionsCount;
		gpuInfos[i].extensionProperties = extensions;
		gpuInfos[i].queueFamilyProperties = queueFamilyProperties;
		gpuInfos[i].queueFamiliesCount = queueFamiliesCount;
	}
}

static int32_t r_getFirstCompatibleGpuIndex(GpuInfo *gpuInfos, uint32_t gpuCount)
{
	uint32_t desiredExtensionsCount = 1;
	const char *const desiredExtensions[1] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	for (uint32_t i = 0; i < gpuCount; i++)
	{
		boolean allExtensionsAvailable = TRUE;
		for (uint32_t j = 0; j < desiredExtensionsCount; j++)
		{
			if (!r_isExtensionAvailable(desiredExtensions[j], gpuInfos[i].extensionProperties, desiredExtensionsCount))
			{
				allExtensionsAvailable = FALSE;
				break;
			}
		}
		if (allExtensionsAvailable)
		{
			return i;
		}
	}
	return -1;
}

static boolean r_isExtensionAvailable(
	const char *const desiredExtension,
	VkExtensionProperties *availableExtensions,
	uint32_t extensionsCount)
{
	for (uint32_t i = 0; i < extensionsCount; i++)
	{
		if (strcmp(desiredExtension, availableExtensions[i].extensionName) == 0)
			return TRUE;
	}
	return FALSE;
}

static void r_createLogicalDevice()
{
	VkResult result;

	const char *const desired_device_extensions[1] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	VkQueueFlags desired_capabilities = VK_QUEUE_GRAPHICS_BIT;

	g_deviceInfo.queueFamiliesCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(g_gpuInfo.device, &g_deviceInfo.queueFamiliesCount, NULL);
	assert(g_deviceInfo.queueFamiliesCount > 0);

	g_deviceInfo.queueFamiliesProperties =
		(VkQueueFamilyProperties*)malloc(g_deviceInfo.queueFamiliesCount * sizeof(VkQueueFamilyProperties)); // TODO: free
	vkGetPhysicalDeviceQueueFamilyProperties(g_gpuInfo.device, &g_deviceInfo.queueFamiliesCount, g_deviceInfo.queueFamiliesProperties);

	g_deviceInfo.presentQueueFamilyIndex = g_deviceInfo.queueFamiliesCount;

	for (uint32_t i = 0; i < g_deviceInfo.queueFamiliesCount; i++)
	{
		if (g_deviceInfo.queueFamiliesProperties[i].queueCount > 0 &&
			(g_deviceInfo.queueFamiliesProperties[i].queueFlags & desired_capabilities) != 0)
		{
			g_deviceInfo.presentQueueFamilyIndex = i;
			break;
		}
	}

	// TODO: assert capable queue family found

	float priority = 1.0f;

	VkDeviceQueueCreateInfo queueCreateInfo = { 0 };
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.pNext = NULL;
	queueCreateInfo.flags = 0;
	queueCreateInfo.queueFamilyIndex = g_deviceInfo.presentQueueFamilyIndex;
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
	deviceCreateInfo.enabledExtensionCount = 1;
	deviceCreateInfo.ppEnabledExtensionNames = desired_device_extensions;
	deviceCreateInfo.pEnabledFeatures = NULL;

	result = vkCreateDevice(g_gpuInfo.device, &deviceCreateInfo, NULL, &g_deviceInfo.device);
	assert(result == VK_SUCCESS);

	vkGetDeviceQueue(g_deviceInfo.device, g_deviceInfo.presentQueueFamilyIndex, 0, &g_deviceInfo.presentQueue);
}

static void r_createPresentCommandPoolAndBuffers()
{
	VkResult result;
	g_presentCommandPoolInfo.commandBufferCount = 1;
	g_presentCommandPoolInfo.commandBuffers = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer));

	VkCommandPoolCreateInfo commandPoolCreateInfo = { 0 };
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.pNext = NULL;
	commandPoolCreateInfo.flags = 0;
	commandPoolCreateInfo.queueFamilyIndex = g_deviceInfo.presentQueueFamilyIndex;

	result = vkCreateCommandPool(g_deviceInfo.device, &commandPoolCreateInfo, NULL, &g_presentCommandPoolInfo.commandPool);
	assert(result == VK_SUCCESS);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = { 0 };
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = NULL;
	commandBufferAllocateInfo.commandPool = g_presentCommandPoolInfo.commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = g_presentCommandPoolInfo.commandBufferCount;

	result = vkAllocateCommandBuffers(g_deviceInfo.device, &commandBufferAllocateInfo, g_presentCommandPoolInfo.commandBuffers);
	assert(result == VK_SUCCESS);
}

// TODO: Stricter (or any..) validation
// TODO: What goes with swapchain creation and what goes with surface creation?
static void r_createPresentationSurface()
{
	VkResult result;
	VkSurfaceKHR presentationSurface = createVkSurface(g_instance, g_osParams.hInstance, g_presentInfo.window);

	VkBool32 surfaceIsSupported;
	result = vkGetPhysicalDeviceSurfaceSupportKHR(g_gpuInfo.device, g_deviceInfo.presentQueueFamilyIndex, presentationSurface, &surfaceIsSupported);
	assert(result == VK_SUCCESS);
	assert(surfaceIsSupported == VK_TRUE);

	g_presentInfo.surface = presentationSurface;

	VkPresentModeKHR desiredPresentMode = VK_PRESENT_MODE_MAILBOX_KHR; // TODO: use it

	g_presentInfo.presentModesCount = 0;
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(g_gpuInfo.device, presentationSurface, &g_presentInfo.presentModesCount, NULL);
	assert(result == VK_SUCCESS);

	g_presentInfo.presentModes =
		(VkPresentModeKHR*)malloc(g_presentInfo.presentModesCount * sizeof(VkPresentModeKHR)); // TODO: free
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(g_gpuInfo.device, g_presentInfo.surface, &g_presentInfo.presentModesCount, g_presentInfo.presentModes);
	assert(result == VK_SUCCESS);

	result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_gpuInfo.device, g_presentInfo.surface, &g_presentInfo.surfaceCapabilities);
	assert(result == VK_SUCCESS);

	// TODO: Some voodoo
	g_presentInfo.imagesCount = g_presentInfo.surfaceCapabilities.minImageCount + 1;
	if (g_presentInfo.surfaceCapabilities.maxImageCount > 0)
	{
		if (g_presentInfo.imagesCount > g_presentInfo.surfaceCapabilities.maxImageCount)
			g_presentInfo.imagesCount = g_presentInfo.surfaceCapabilities.maxImageCount;
	}

	// TODO: ...what?
	if ((uint32_t)g_presentInfo.surfaceCapabilities.currentExtent.width == -1)
	{
		// TODO: implement case
	}
	else
	{
		// size_of_images = surface_capabilities.currentExtent;
	}

	g_presentInfo.imagesExtent = g_presentInfo.surfaceCapabilities.currentExtent;

	// TODO: check desired/supported usages.

	VkImageUsageFlags desiredUsages = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	// TODO: set desired transformation of swapchain images

	result = vkGetPhysicalDeviceSurfaceFormatsKHR(g_gpuInfo.device, g_presentInfo.surface, &g_presentInfo.surfaceFormatsCount, NULL);
	assert(result == VK_SUCCESS);

	g_presentInfo.surfaceFormats =
		(VkSurfaceFormatKHR*)malloc(g_presentInfo.surfaceFormatsCount * sizeof(VkSurfaceFormatKHR)); // TODO: free
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(g_gpuInfo.device, g_presentInfo.surface, &g_presentInfo.surfaceFormatsCount, g_presentInfo.surfaceFormats);
	assert(result == VK_SUCCESS);
}

// TODO: What goes with swapchain creation and what goes with surface creation?
// TODO: Maybe split 
// TODO: Validation
static void r_createPresentationSwapchain()
{
	VkResult result;

	g_swapchainInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
	g_swapchainInfo.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	VkImageUsageFlags imageUsages = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	VkSwapchainCreateInfoKHR swapchainCreateInfo = { 0 };
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.pNext = NULL;
	swapchainCreateInfo.flags = 0;
	swapchainCreateInfo.surface = g_presentInfo.surface;
	swapchainCreateInfo.minImageCount = g_presentInfo.imagesCount;
	swapchainCreateInfo.imageFormat = g_swapchainInfo.format;
	swapchainCreateInfo.imageColorSpace = g_swapchainInfo.colorSpace;
	swapchainCreateInfo.imageExtent = g_presentInfo.imagesExtent;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = imageUsages;
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.queueFamilyIndexCount = 0;
	swapchainCreateInfo.pQueueFamilyIndices = NULL;
	swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR; // TODO: set it up
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = g_presentInfo.presentModes[0]; // TODO: set it up
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	result = vkCreateSwapchainKHR(g_deviceInfo.device, &swapchainCreateInfo, NULL, &g_swapchainInfo.swapchain);
	assert(result == VK_SUCCESS);
}

static void r_getPresentationSwapchainImages()
{
	VkResult result;

	result = vkGetSwapchainImagesKHR(g_deviceInfo.device, g_swapchainInfo.swapchain, &g_swapchainInfo.imagesCount, NULL);
	assert(result == VK_SUCCESS);

	g_swapchainInfo.images =
		(VkImage*)malloc(g_swapchainInfo.imagesCount * sizeof(VkImage)); // TODO: free

	result = vkGetSwapchainImagesKHR(g_deviceInfo.device, g_swapchainInfo.swapchain, &g_swapchainInfo.imagesCount, g_swapchainInfo.images);
	assert(result == VK_SUCCESS);
}

static void r_allocatePresentationSwapchainImageMemory()
{
	VkResult result;

	g_swapchainInfo.imageMemoryRequirements =
		(VkMemoryRequirements*)malloc(g_swapchainInfo.imagesCount * sizeof(VkMemoryRequirements)); // TODO: free
	for (uint32_t i = 0; i < g_swapchainInfo.imagesCount; i++)
	{
		vkGetImageMemoryRequirements(g_deviceInfo.device, g_swapchainInfo.images[i], &g_swapchainInfo.imageMemoryRequirements[i]);
	}

	g_swapchainInfo.imageMemory =
		(VkDeviceMemory*)malloc(g_swapchainInfo.imagesCount * sizeof(VkDeviceMemory)); // TODO: free
	for (uint32_t i = 0; i < g_swapchainInfo.imagesCount; i++)
	{
		g_swapchainInfo.imageMemory[i] = VK_NULL_HANDLE;
	}

	// TODO: this is a mess
	for (uint32_t i = 0; i < g_swapchainInfo.imagesCount; i++)
	{
		for (uint32_t j = 1; j < g_gpuInfo.memoryProperties.memoryTypeCount; j++)
		{
			if ((g_swapchainInfo.imageMemoryRequirements[i].memoryTypeBits & (1 << j)))
			{
				VkMemoryAllocateInfo imageMemoryAllocateInfo = { 0 };
				imageMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				imageMemoryAllocateInfo.pNext = NULL;
				imageMemoryAllocateInfo.allocationSize = g_swapchainInfo.imageMemoryRequirements[i].size;
				imageMemoryAllocateInfo.memoryTypeIndex = j;

				result = vkAllocateMemory(g_deviceInfo.device, &imageMemoryAllocateInfo, NULL, &g_swapchainInfo.imageMemory[i]);
				assert(result == VK_SUCCESS);
				break;
			}
		}
	}
}

static void r_createPresentationSwapchainImageViews()
{
	VkResult result;

	g_swapchainInfo.imageSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	g_swapchainInfo.imageSubresourceRange.baseMipLevel = 0;
	g_swapchainInfo.imageSubresourceRange.levelCount = 1;
	g_swapchainInfo.imageSubresourceRange.baseArrayLayer = 0;
	g_swapchainInfo.imageSubresourceRange.layerCount = 1;

	VkImageViewCreateInfo *imageViewCreateInfos =
		(VkImageViewCreateInfo*)malloc(g_swapchainInfo.imagesCount * sizeof(VkImageViewCreateInfo)); // TODO: free
	g_swapchainInfo.imageViews =
		(VkImageView*)malloc(g_swapchainInfo.imagesCount * sizeof(VkImageView)); // TODO: free
	for (uint32_t i = 0; i < g_swapchainInfo.imagesCount; i++)
	{
		imageViewCreateInfos[i].sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfos[i].pNext = NULL;
		imageViewCreateInfos[i].flags = 0;
		imageViewCreateInfos[i].image = g_swapchainInfo.images[i];
		imageViewCreateInfos[i].viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfos[i].format = g_swapchainInfo.format;
		imageViewCreateInfos[i].components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfos[i].components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfos[i].components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfos[i].components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfos[i].subresourceRange = g_swapchainInfo.imageSubresourceRange;

		result = vkCreateImageView(g_deviceInfo.device, &imageViewCreateInfos[i], NULL, &g_swapchainInfo.imageViews[i]);
		assert(result == VK_SUCCESS);
	}
}

static void r_createPresentationSynchronizationPrimitives()
{
	VkResult result;

	VkSemaphoreCreateInfo semaphoreCreateInfo = { 0 };
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = NULL;
	semaphoreCreateInfo.flags = 0;

	result = vkCreateSemaphore(g_deviceInfo.device, &semaphoreCreateInfo, NULL, &g_presentInfo.semaphore);
	assert(result == VK_SUCCESS);

	VkFenceCreateInfo fenceCreateInfo = { 0 };
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.pNext = NULL;
	fenceCreateInfo.flags = 0;

	result = vkCreateFence(g_deviceInfo.device, &fenceCreateInfo, NULL, &g_presentInfo.fence);
	assert(result == VK_SUCCESS);
}

static void r_initImageLayouts()
{
	r_showWindow();

	VkResult result;
	result = vkAcquireNextImageKHR(g_deviceInfo.device, g_swapchainInfo.swapchain, 2000000000, g_presentInfo.semaphore, NULL, &g_swapchainInfo.nextImageIndex);

	r_updateImageTransitionMemoryBarriers();

	VkCommandBufferBeginInfo commandBufferBeginInfo = { 0 };
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = NULL;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	commandBufferBeginInfo.pInheritanceInfo = NULL;

	g_presentInfo.clearColor.float32[0] = 0.5f;
	g_presentInfo.clearColor.float32[1] = 0.5f;
	g_presentInfo.clearColor.float32[2] = 1.0f;
	g_presentInfo.clearColor.float32[3] = 0.0f;


	result = vkBeginCommandBuffer(g_presentCommandPoolInfo.commandBuffers[0], &commandBufferBeginInfo);
	assert(result == VK_SUCCESS);

	vkCmdPipelineBarrier(g_presentCommandPoolInfo.commandBuffers[0],
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
		0, 0, NULL, 0, NULL, 1, &g_imageTransitions.undefinedToTransfer);
	g_imageTransitions.currentLayout = g_imageTransitions.undefinedToTransfer;

	vkCmdClearColorImage(g_presentCommandPoolInfo.commandBuffers[0], g_swapchainInfo.images[g_swapchainInfo.nextImageIndex],
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &g_presentInfo.clearColor,
		1, &g_swapchainInfo.imageSubresourceRange);

	vkCmdPipelineBarrier(g_presentCommandPoolInfo.commandBuffers[0],
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		0, 0, NULL, 0, NULL, 1, &g_imageTransitions.transferToPresent);
	g_imageTransitions.currentLayout = g_imageTransitions.transferToPresent;

	result = vkEndCommandBuffer(g_presentCommandPoolInfo.commandBuffers[0]);
	assert(result == VK_SUCCESS);

	// TODO: fix semaphore business
	VkSubmitInfo submitInfo = { 0 };
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = NULL;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = NULL;
	submitInfo.pWaitDstStageMask = NULL;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &g_presentCommandPoolInfo.commandBuffers[0];
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = NULL;

	result = vkQueueSubmit(g_deviceInfo.presentQueue, 1, &submitInfo, VK_NULL_HANDLE);

	VkPresentInfoKHR presentInfo = { 0 };
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = NULL;
	presentInfo.waitSemaphoreCount = 0;
	presentInfo.pWaitSemaphores = NULL;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &g_swapchainInfo.swapchain;
	presentInfo.pImageIndices = &g_swapchainInfo.nextImageIndex;

	result = vkQueuePresentKHR(g_deviceInfo.presentQueue, &presentInfo);
}

static inline void r_updateImageTransitionMemoryBarriers()
{
	g_imageTransitions.undefinedToTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	g_imageTransitions.undefinedToTransfer.pNext = NULL;
	g_imageTransitions.undefinedToTransfer.srcAccessMask = 0;
	g_imageTransitions.undefinedToTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	g_imageTransitions.undefinedToTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	g_imageTransitions.undefinedToTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	g_imageTransitions.undefinedToTransfer.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	g_imageTransitions.undefinedToTransfer.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	g_imageTransitions.undefinedToTransfer.image = g_swapchainInfo.images[g_swapchainInfo.nextImageIndex];
	g_imageTransitions.undefinedToTransfer.subresourceRange = g_swapchainInfo.imageSubresourceRange;

	g_imageTransitions.transferToPresent.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	g_imageTransitions.transferToPresent.pNext = NULL;
	g_imageTransitions.transferToPresent.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	g_imageTransitions.transferToPresent.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	g_imageTransitions.transferToPresent.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	g_imageTransitions.transferToPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	g_imageTransitions.transferToPresent.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	g_imageTransitions.transferToPresent.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	g_imageTransitions.transferToPresent.image = g_swapchainInfo.images[g_swapchainInfo.nextImageIndex];
	g_imageTransitions.transferToPresent.subresourceRange = g_swapchainInfo.imageSubresourceRange;
}

static void r_readShaders()
{
	FILE *vertexShader;
	assert(fopen_s(&vertexShader, "vert.spv", "rb") != EINVAL);

	fseek(vertexShader, 0, SEEK_END);
	size_t vertexShaderFileSize = ftell(vertexShader);
	fseek(vertexShader, 0, SEEK_SET);

	g_shaders.vertexShaderFileSize = vertexShaderFileSize;
	g_shaders.vertexShader = (char*)malloc(vertexShaderFileSize + 1);

	fread_s(g_shaders.vertexShader, vertexShaderFileSize + 1, 1, vertexShaderFileSize, vertexShader);

	fclose(vertexShader);

	FILE *fragShader;
	assert(fopen_s(&fragShader, "frag.spv", "rb") != EINVAL);

	fseek(fragShader, 0, SEEK_END);
	size_t fragShaderFileSize = ftell(fragShader);
	fseek(fragShader, 0, SEEK_SET);

	g_shaders.fragmentShaderFileSize = fragShaderFileSize;
	g_shaders.fragmentShader = (char*)malloc(fragShaderFileSize + 1);

	fread_s(g_shaders.fragmentShader, fragShaderFileSize + 1, 1, fragShaderFileSize, fragShader);

	fclose(fragShader);
}

static void r_createDefaultShaderModules()
{
	VkResult result;

	VkShaderModuleCreateInfo vertexShaderCreateInfo = { 0 };
	vertexShaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vertexShaderCreateInfo.pNext = NULL;
	vertexShaderCreateInfo.flags = 0;
	vertexShaderCreateInfo.codeSize = g_shaders.vertexShaderFileSize;
	vertexShaderCreateInfo.pCode = (uint32_t*)g_shaders.vertexShader;

	result = vkCreateShaderModule(g_deviceInfo.device, &vertexShaderCreateInfo, NULL, &g_graphicsPipeline.vertShaderModule);
	assert(result == VK_SUCCESS);

	VkShaderModuleCreateInfo fragmentShaderCreateInfo = { 0 };
	fragmentShaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	fragmentShaderCreateInfo.pNext = NULL;
	fragmentShaderCreateInfo.flags = 0;
	fragmentShaderCreateInfo.codeSize = g_shaders.fragmentShaderFileSize;
	fragmentShaderCreateInfo.pCode = (uint32_t*)g_shaders.fragmentShader;

	result = vkCreateShaderModule(g_deviceInfo.device, &fragmentShaderCreateInfo, NULL, &g_graphicsPipeline.fragShaderModule);
	assert(result == VK_SUCCESS);
}

static void r_createRenderPass()
{
	VkResult result;

	VkAttachmentDescription colorAttachment = { 0 };
	colorAttachment.flags = 0;
	colorAttachment.format = g_swapchainInfo.format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

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

	VkRenderPassCreateInfo renderPassCreateInfo = { 0 };
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.pNext = NULL;
	renderPassCreateInfo.flags = 0;
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = &colorAttachment;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassDescription;
	renderPassCreateInfo.dependencyCount = 1;
	renderPassCreateInfo.pDependencies = &subpassDependency;

	result = vkCreateRenderPass(g_deviceInfo.device, &renderPassCreateInfo, NULL, &g_graphicsPipeline.renderPass);
	assert(result == VK_SUCCESS);
}

static void r_createDefaultGraphicsPipeline()
{
	VkResult result;

	VkPipelineShaderStageCreateInfo shaderStages[2];
	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].pNext = NULL;
	shaderStages[0].flags = 0;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = g_graphicsPipeline.vertShaderModule;
	shaderStages[0].pName = "main";
	shaderStages[0].pSpecializationInfo = NULL;

	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].pNext = NULL;
	shaderStages[1].flags = 0;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module = g_graphicsPipeline.fragShaderModule;
	shaderStages[1].pName = "main";
	shaderStages[1].pSpecializationInfo = NULL;


	VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = { 0 };
	pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	pipelineVertexInputStateCreateInfo.pNext = NULL;
	pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
	pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = NULL;
	pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
	pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = NULL;

	VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo = { 0 };
	pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pipelineInputAssemblyStateCreateInfo.pNext = NULL;
	pipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = { 0 };
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = g_presentInfo.imagesExtent.width; // TODO: ...isn't this NDC?
	viewport.height = g_presentInfo.imagesExtent.height; // TODO: ...isn't this NDC?
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = { 0 };
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent = g_presentInfo.imagesExtent;

	VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo = { 0 };
	pipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	pipelineViewportStateCreateInfo.pNext = NULL;
	pipelineViewportStateCreateInfo.flags = 0;
	pipelineViewportStateCreateInfo.viewportCount = 1;
	pipelineViewportStateCreateInfo.pViewports = &viewport;
	pipelineViewportStateCreateInfo.scissorCount = 1;
	pipelineViewportStateCreateInfo.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo = { 0 };
	pipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	pipelineRasterizationStateCreateInfo.pNext = NULL;
	pipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
	pipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	pipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;
	pipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	pipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	pipelineRasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
	pipelineRasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
	pipelineRasterizationStateCreateInfo.depthBiasClamp = 0.0f;
	pipelineRasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo = { 0 };
	pipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	pipelineMultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
	pipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	pipelineMultisampleStateCreateInfo.minSampleShading = 1.0f;
	pipelineMultisampleStateCreateInfo.pSampleMask = NULL;
	pipelineMultisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
	pipelineMultisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState = { 0 };
	pipelineColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_R_BIT;
	pipelineColorBlendAttachmentState.blendEnable = VK_FALSE;
	// Not used
	pipelineColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	pipelineColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	pipelineColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	pipelineColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	pipelineColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	pipelineColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo = { 0 };
	pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	pipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
	pipelineColorBlendStateCreateInfo.attachmentCount = 1;
	pipelineColorBlendStateCreateInfo.pAttachments = &pipelineColorBlendAttachmentState;
	// Not used
	pipelineColorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
	pipelineColorBlendStateCreateInfo.blendConstants[0] = 0.0f;
	pipelineColorBlendStateCreateInfo.blendConstants[1] = 0.0f;
	pipelineColorBlendStateCreateInfo.blendConstants[2] = 0.0f;
	pipelineColorBlendStateCreateInfo.blendConstants[3] = 0.0f;

	// TODO: Dynamic states

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = { 0 };
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pNext = NULL;
	pipelineLayoutCreateInfo.flags = 0;
	pipelineLayoutCreateInfo.setLayoutCount = 0;
	pipelineLayoutCreateInfo.pSetLayouts = NULL;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = NULL;

	result = vkCreatePipelineLayout(g_deviceInfo.device, &pipelineLayoutCreateInfo, NULL, &g_graphicsPipeline.layout);
	assert(result == VK_SUCCESS);

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = { 0 };
	graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineCreateInfo.pNext = NULL;
	graphicsPipelineCreateInfo.flags = 0;
	graphicsPipelineCreateInfo.stageCount = 2;
	graphicsPipelineCreateInfo.pStages = shaderStages;
	graphicsPipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;
	graphicsPipelineCreateInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
	graphicsPipelineCreateInfo.pTessellationState = NULL;
	graphicsPipelineCreateInfo.pViewportState = &pipelineViewportStateCreateInfo;
	graphicsPipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
	graphicsPipelineCreateInfo.pMultisampleState = &pipelineMultisampleStateCreateInfo;
	graphicsPipelineCreateInfo.pDepthStencilState = NULL;
	graphicsPipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;
	graphicsPipelineCreateInfo.pDynamicState = NULL;
	graphicsPipelineCreateInfo.layout = g_graphicsPipeline.layout;
	graphicsPipelineCreateInfo.renderPass = g_graphicsPipeline.renderPass;
	graphicsPipelineCreateInfo.subpass = 0;
	graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	graphicsPipelineCreateInfo.basePipelineIndex = -1;

	result = vkCreateGraphicsPipelines(g_deviceInfo.device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, NULL, &g_graphicsPipeline.pipeline);
	assert(result == VK_SUCCESS);
}

static void r_createFrameBuffers()
{
	VkResult result;

	g_swapchainInfo.framebuffers =
		(VkFramebuffer*)malloc(g_swapchainInfo.imagesCount * sizeof(VkFramebuffer)); // TODO: free

	for (uint32_t i = 0; i < g_swapchainInfo.imagesCount; i++)
	{
		VkFramebufferCreateInfo framebufferCreateInfo = { 0 };
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.pNext = NULL;
		framebufferCreateInfo.flags = 0;
		framebufferCreateInfo.renderPass = g_graphicsPipeline.renderPass;
		framebufferCreateInfo.attachmentCount = 1;
		framebufferCreateInfo.pAttachments = &g_swapchainInfo.imageViews[i];
		framebufferCreateInfo.width = g_presentInfo.imagesExtent.width;
		framebufferCreateInfo.height = g_presentInfo.imagesExtent.height;
		framebufferCreateInfo.layers = 1;

		result = vkCreateFramebuffer(g_deviceInfo.device, &framebufferCreateInfo, NULL, &g_swapchainInfo.framebuffers[i]);
		assert(result == VK_SUCCESS);
	}

}

void r_renderFrame()
{
	VkResult result;
	result = vkAcquireNextImageKHR(g_deviceInfo.device, g_swapchainInfo.swapchain, 2000000000, g_presentInfo.semaphore, NULL, &g_swapchainInfo.nextImageIndex);

	r_updateImageTransitionMemoryBarriers();

	VkCommandBufferBeginInfo commandBufferBeginInfo = { 0 };
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = NULL;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	commandBufferBeginInfo.pInheritanceInfo = NULL;

	VkRenderPassBeginInfo renderPassBeginInfo = { 0 };
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.pNext = NULL;
	renderPassBeginInfo.renderPass = g_graphicsPipeline.renderPass;
	renderPassBeginInfo.framebuffer = g_swapchainInfo.framebuffers[g_swapchainInfo.nextImageIndex];
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent = g_presentInfo.imagesExtent;
	renderPassBeginInfo.clearValueCount = 1;
	renderPassBeginInfo.pClearValues = &g_presentInfo.clearColor;

	result = vkBeginCommandBuffer(g_presentCommandPoolInfo.commandBuffers[0], &commandBufferBeginInfo);
	assert(result == VK_SUCCESS);

	vkCmdBeginRenderPass(g_presentCommandPoolInfo.commandBuffers[0], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(g_presentCommandPoolInfo.commandBuffers[0], VK_PIPELINE_BIND_POINT_GRAPHICS, g_graphicsPipeline.pipeline);
	vkCmdDraw(g_presentCommandPoolInfo.commandBuffers[0], 3, 1, 0, 0);
	vkCmdEndRenderPass(g_presentCommandPoolInfo.commandBuffers[0]);
	
	result = vkEndCommandBuffer(g_presentCommandPoolInfo.commandBuffers[0]);
	assert(result == VK_SUCCESS);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void r_handleOSMessages()
{
	// TODO: Peek message
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

static HWND createAndRegisterWindow(HINSTANCE hInstance)
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

	// TODO: clean up
	HWND window = CreateWindow(
		L"windowClass", // class name
		L"vulkan", // window name
		WS_OVERLAPPEDWINDOW, // TODO: consider flags
		CW_USEDEFAULT, // pos x
		CW_USEDEFAULT, // pos y
		500, // width
		100, // height
		NULL,
		NULL,
		hInstance,
		NULL
	);

	return window;
}