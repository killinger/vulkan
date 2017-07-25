#define VK_USE_PLATFORM_WIN32_KHR

#include <Windows.h>
#include <vulkan\vulkan.h>
#include <stdio.h>
#include <tchar.h>
#include <assert.h>
#include "renderer.h"
#include "vulkaninfo.h"
#include "simd_math.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

typedef struct Vertex {
	float pos[2];
	float color[3];
} Vertex;

struct Camera
{
	float x;
	float y;
	Matrix4x4 projection;
};

// test
// top = -1
// left = -1
static struct Vertex test_vertices[3] = {
	{ {0.0f, 50.0f}, {0.3f, 0.0f, 0.9f} },
	{ {50.0f, 0.0f}, {0.4f, 0.0f, 0.8f} },
	{ {-50.0f, 0.0f}, {0.5f, 0.0f, 0.7f} }
};

static struct Vertex test_quad[6] = {
	{{-25.0f, 50.0f }, { 0.6f, 0.0f, 0.9f } },
	{{25.0f, 0.0f}, { 0.3f, 0.0f, 0.9f } },
	{{-25.0f, 0.0f }, { 0.3f, 0.0f, 0.9f } },
	{{-25.0f, 50.0f }, { 0.6f, 0.0f, 0.9f } },
	{{ 25.0f, 50.0f }, { 0.6f, 0.0f, 0.9f } },
	{{25.0f, 0.0f }, {0.3f, 0.0f, 0.9f} }
};

static struct Camera camera;

// File scope functions
// TODO: fix conventions
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
static void r_initImageLayouts(); // TODO: get out?
static void r_updateImageTransitionMemoryBarriers();
static VkVertexInputBindingDescription r_createVertexBindingDescription(uint32_t binding); // TODO: test
static VkVertexInputAttributeDescription r_createPositionVertexAttributeDescription( // TODO: test
	uint32_t binding, uint32_t location);
static VkVertexInputAttributeDescription r_createColorVertexAttributeDescription( // TODO: test
	uint32_t binding, uint32_t location);
static void r_createVertexBuffer(); // TODO: test
static void r_createUniformBuffer();
static void r_createTexture();
static void r_readShaders();
static void r_createDefaultShaderModules();
static void r_createRenderPass();
static void r_createDescriptorSetLayout();
static void r_createDescriptorPool();
static void r_createDescriptorSet();
static void r_createDefaultGraphicsPipeline();
static void r_createFrameBuffers();
static void r_readImageFile(const char *file, Texture *texture);
static void r_createTexture();
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
static DescriptorInfo g_descriptorInfo = { 0 };
static GraphicsPipeline g_graphicsPipeline = { 0 };

static VertexBuffer g_vertexBuffer = { 0 };
static UniformBuffer g_uniformBuffer = { 0 };

void r_initVulkan(OsParams params)
{
	//createOrthographicProjectionMatrix(&camera.orthographicProjection, -1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 10.0f);
	//createOrthographicProjectionMatrix(&camera.projection, 0.0f, 1280.0f, 0.0f, 720.0f, 0.1f, 10.0f);

	camera.projection = createOrthographicProjectionMatrix(-1280.0f / 2, 1280.0f / 2, -720.0f / 2, 720.0f / 2, 0.1f, 10.0f);
	camera.x = 2.0f;
	camera.y = 2.0f;

	Matrix4x4 cameraTranslation = createTranslationMatrix(-camera.x, -camera.y, 0.0f);
	Matrix4x4 cameraBasis = createMatrixSetDiagonals(1.0f, -1.0f, -1.0f, 1.0f);
	Matrix4x4 cameraTransform = multiply(cameraBasis, cameraTranslation);
	Matrix4x4 vpMatrix = multiply(camera.projection, cameraTransform);

	g_osParams = params;
	r_createOSWindow();
	r_showWindow();
	g_instance = r_createVkInstance();
	g_gpuInfo = r_findFirstCompatibleGpu();
	r_createLogicalDevice();
	r_createPresentationSurface();
	r_createPresentationSwapchain();
	r_getPresentationSwapchainImages();
	r_allocatePresentationSwapchainImageMemory();
	r_createPresentationSwapchainImageViews();
	r_createPresentationSynchronizationPrimitives();
	//r_initImageLayouts();
	r_createVertexBuffer();
	r_createUniformBuffer();
	//
	r_readShaders();
	r_createDefaultShaderModules();
	r_createRenderPass();
	r_createDescriptorSetLayout();
	r_createDescriptorPool();
	r_createDescriptorSet();
	r_createDefaultGraphicsPipeline();
	r_createFrameBuffers();
	r_createPresentCommandPoolAndBuffers();
	r_createTexture();
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
	g_presentCommandPoolInfo.commandBufferCount = g_swapchainInfo.imagesCount;
	g_presentCommandPoolInfo.commandBuffers =
		(VkCommandBuffer*)malloc(g_presentCommandPoolInfo.commandBufferCount * sizeof(VkCommandBuffer)); // TODO: free

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

	// TODO: maybe one commandbuffer per swapchain image?

	VkCommandBufferBeginInfo commandBufferBeginInfo = { 0 };
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = NULL;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	commandBufferBeginInfo.pInheritanceInfo = NULL;

	VkRenderPassBeginInfo *renderPassBeginInfos =
		(VkRenderPassBeginInfo*)malloc(g_swapchainInfo.imagesCount * sizeof(VkRenderPassBeginInfo)); // TODO: free

	VkDeviceSize deviceSizeOffsets = { 0 };

	for (uint32_t i = 0; i < g_swapchainInfo.imagesCount; i++)
	{
		renderPassBeginInfos[i].sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfos[i].pNext = NULL;
		renderPassBeginInfos[i].renderPass = g_graphicsPipeline.renderPass;
		renderPassBeginInfos[i].framebuffer = g_swapchainInfo.framebuffers[i];
		renderPassBeginInfos[i].renderArea.offset.x = 0;
		renderPassBeginInfos[i].renderArea.offset.y = 0;
		renderPassBeginInfos[i].renderArea.extent = g_presentInfo.imagesExtent;
		renderPassBeginInfos[i].clearValueCount = 1;
		renderPassBeginInfos[i].pClearValues = (const VkClearValue*)&g_presentInfo.clearColor;

		result = vkBeginCommandBuffer(g_presentCommandPoolInfo.commandBuffers[i], &commandBufferBeginInfo);
		assert(result == VK_SUCCESS);
		{
			vkCmdBeginRenderPass(g_presentCommandPoolInfo.commandBuffers[i], &renderPassBeginInfos[i], VK_SUBPASS_CONTENTS_INLINE);
			{
				vkCmdBindPipeline(g_presentCommandPoolInfo.commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, g_graphicsPipeline.pipeline);
				vkCmdBindVertexBuffers(g_presentCommandPoolInfo.commandBuffers[i], 0, 1, &g_vertexBuffer.buffer, &deviceSizeOffsets);
				vkCmdBindDescriptorSets(g_presentCommandPoolInfo.commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
					g_graphicsPipeline.layout, 0, 1, &g_descriptorInfo.set, 0, NULL);
				vkCmdDraw(g_presentCommandPoolInfo.commandBuffers[i], sizeof(test_quad) / sizeof(struct Vertex), 1, 0, 0);
			}
			vkCmdEndRenderPass(g_presentCommandPoolInfo.commandBuffers[i]);
		}
		result = vkEndCommandBuffer(g_presentCommandPoolInfo.commandBuffers[i]);
		assert(result == VK_SUCCESS);
	}
}

// TODO: Stricter (or any..) validation
// TODO: What goes with swapchain creation and what goes with surface creation?
static void r_createPresentationSurface()
{
	VkResult result;

	VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfo = { 0 };
	win32SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	win32SurfaceCreateInfo.pNext = NULL;
	win32SurfaceCreateInfo.flags = 0;
	win32SurfaceCreateInfo.hinstance = g_osParams.hInstance;
	win32SurfaceCreateInfo.hwnd = g_presentInfo.window;

	VkSurfaceKHR presentationSurface;
	result = vkCreateWin32SurfaceKHR(g_instance, &win32SurfaceCreateInfo, NULL, &presentationSurface);
	assert(result == VK_SUCCESS);

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
		(VkImageViewCreateInfo*)malloc(g_swapchainInfo.imagesCount * sizeof(VkImageViewCreateInfo));
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

	free(imageViewCreateInfos);
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
	VkResult result;
	result = vkAcquireNextImageKHR(g_deviceInfo.device, g_swapchainInfo.swapchain, 2000000000, g_presentInfo.semaphore, VK_NULL_HANDLE, &g_swapchainInfo.nextImageIndex);

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

	VkPipelineStageFlags submitWaitPipelineStageFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

	// TODO: fix semaphore business
	VkSubmitInfo submitInfo = { 0 };
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = NULL;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &g_presentInfo.semaphore;
	submitInfo.pWaitDstStageMask = &submitWaitPipelineStageFlags;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &g_presentCommandPoolInfo.commandBuffers[0];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &g_presentInfo.semaphore;

	result = vkQueueSubmit(g_deviceInfo.presentQueue, 1, &submitInfo, VK_NULL_HANDLE);

	VkPresentInfoKHR presentInfo = { 0 };
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = NULL;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &g_presentInfo.semaphore;
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

static VkVertexInputBindingDescription r_createVertexBindingDescription(uint32_t binding)
{
	VkVertexInputBindingDescription vertexInputBindingDescription = { 0 };
	vertexInputBindingDescription.binding = binding;
	vertexInputBindingDescription.stride = sizeof(struct Vertex);
	vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return vertexInputBindingDescription;
}

static VkVertexInputAttributeDescription r_createPositionVertexAttributeDescription(
	uint32_t binding, uint32_t location)
{
	VkVertexInputAttributeDescription vertexInputAttributeDescription = { 0 };
	vertexInputAttributeDescription.binding = binding;
	vertexInputAttributeDescription.location = location;
	vertexInputAttributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
	vertexInputAttributeDescription.offset = offsetof(struct Vertex, pos);

	return vertexInputAttributeDescription;
}

static VkVertexInputAttributeDescription r_createColorVertexAttributeDescription(
	uint32_t binding, uint32_t location)
{
	VkVertexInputAttributeDescription vertexInputAttributeDescription = { 0 };
	vertexInputAttributeDescription.binding = binding;
	vertexInputAttributeDescription.location = location;
	vertexInputAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributeDescription.offset = offsetof(struct Vertex, color);

	return vertexInputAttributeDescription;
}

static void r_createVertexBuffer()
{
	VkResult result;

	VkBufferCreateInfo vertexBufferCreateInfo = { 0 };
	vertexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertexBufferCreateInfo.pNext = NULL;
	vertexBufferCreateInfo.flags = 0;
	vertexBufferCreateInfo.size = sizeof(test_quad);
	vertexBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	vertexBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	vertexBufferCreateInfo.queueFamilyIndexCount = 1;
	vertexBufferCreateInfo.pQueueFamilyIndices = &g_deviceInfo.presentQueueFamilyIndex;

	result = vkCreateBuffer(g_deviceInfo.device, &vertexBufferCreateInfo, NULL, &g_vertexBuffer.buffer);

	vkGetBufferMemoryRequirements(g_deviceInfo.device, g_vertexBuffer.buffer, &g_vertexBuffer.memoryRequirements);

	// TODO: Finding suitable memory should be a general purpose function
	int32_t memoryPropertyIndex = -1;
	for (uint32_t i = 0; i < g_gpuInfo.memoryProperties.memoryTypeCount; i++)
	{
		if ((g_vertexBuffer.memoryRequirements.memoryTypeBits & (1 << i)) && // the fuck?
			(g_gpuInfo.memoryProperties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
			== (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
		{
			memoryPropertyIndex = (int32_t)i;
		}
	}

	assert(memoryPropertyIndex >= 0);

	VkMemoryAllocateInfo memoryAllocateInfo = { 0 };
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = NULL;
	memoryAllocateInfo.allocationSize = g_vertexBuffer.memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = memoryPropertyIndex;

	result = vkAllocateMemory(g_deviceInfo.device, &memoryAllocateInfo, NULL, &g_vertexBuffer.memory);
	assert(result == VK_SUCCESS);

	result = vkBindBufferMemory(g_deviceInfo.device, g_vertexBuffer.buffer, g_vertexBuffer.memory, 0);
	assert(result == VK_SUCCESS);

	void *data;
	result = vkMapMemory(g_deviceInfo.device, g_vertexBuffer.memory, 0, sizeof(test_quad), 0, &data);
	assert(result == VK_SUCCESS);

	memcpy(data, test_quad, sizeof(test_quad));
	vkUnmapMemory(g_deviceInfo.device, g_vertexBuffer.memory);

	// TODO: maybe flush?
}

// TODO: general purpose buffer create functions
static void r_createUniformBuffer()
{
	VkResult result;

	camera.projection = createOrthographicProjectionMatrix(-1280.0f / 2, 1280.0f / 2, -720.0f / 2, 720.0f / 2, 0.1f, 10.0f);
	camera.x = 0.0f;
	camera.y = 0.0f;

	Matrix4x4 cameraTranslation = createTranslationMatrix(-camera.x, -camera.y, 0.0f);
	Matrix4x4 cameraBasis = createMatrixSetDiagonals(1.0f, -1.0f, -1.0f, 1.0f);
	Matrix4x4 cameraTransform = multiply(cameraBasis, cameraTranslation);
	Matrix4x4 vpMatrix = multiply(camera.projection, cameraTransform);

	Matrix4x4 scalingMatrix = createScalingMatrixXY(12.0f, 12.0f);
	Matrix4x4 quadTranslation = createTranslationMatrix(0.0f, -300.0f, 0.0f);
	Matrix4x4 modelTransform = multiply(quadTranslation, scalingMatrix);
	vpMatrix = multiply(vpMatrix, modelTransform);

	VkDeviceSize uniformBufferSize = 16 * sizeof(float);

	VkBufferCreateInfo uniformBufferCreateInfo = { 0 };
	uniformBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	uniformBufferCreateInfo.pNext = NULL;
	uniformBufferCreateInfo.flags = 0;
	uniformBufferCreateInfo.size = uniformBufferSize;
	uniformBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	uniformBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	uniformBufferCreateInfo.queueFamilyIndexCount = 1;
	uniformBufferCreateInfo.pQueueFamilyIndices = &g_deviceInfo.presentQueueFamilyIndex;

	result = vkCreateBuffer(g_deviceInfo.device, &uniformBufferCreateInfo, NULL, &g_uniformBuffer.buffer);
	assert(result == VK_SUCCESS);

	vkGetBufferMemoryRequirements(g_deviceInfo.device, g_uniformBuffer.buffer, &g_uniformBuffer.memoryRequirements);

	// TODO: Finding suitable memory should be a general purpose function
	int32_t memoryPropertyIndex = -1;
	for (uint32_t i = 0; i < g_gpuInfo.memoryProperties.memoryTypeCount; i++)
	{
		if ((g_uniformBuffer.memoryRequirements.memoryTypeBits & (1 << i)) && // the fuck?
			(g_gpuInfo.memoryProperties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
			== (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
		{
			memoryPropertyIndex = (int32_t)i;
		}
	}

	assert(memoryPropertyIndex >= 0);

	VkMemoryAllocateInfo memoryAllocateInfo = { 0 };
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = NULL;
	memoryAllocateInfo.allocationSize = g_uniformBuffer.memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = memoryPropertyIndex;

	result = vkAllocateMemory(g_deviceInfo.device, &memoryAllocateInfo, NULL, &g_uniformBuffer.memory);
	assert(result == VK_SUCCESS);

	result = vkBindBufferMemory(g_deviceInfo.device, g_uniformBuffer.buffer, g_uniformBuffer.memory, 0);
	assert(result == VK_SUCCESS);

	void *data;
	result = vkMapMemory(g_deviceInfo.device, g_uniformBuffer.memory, 0, 16 * sizeof(float), 0, &data);
	assert(result == VK_SUCCESS);

	memcpy(data, vpMatrix.e, 16 * sizeof(float));
	vkUnmapMemory(g_deviceInfo.device, g_uniformBuffer.memory);

	// TODO: maybe flush?
}

static void r_readImageFile(const char *file, Texture *texture)
{
	uint32_t componentsCount = 0;

	unsigned char *stbiData = stbi_load(file, &texture->width, &texture->height, &componentsCount, STBI_rgb_alpha);
	assert(componentsCount > 0);
	texture->dataSize = texture->width * texture->height * 4;
	texture->data = (unsigned char*)malloc(texture->dataSize);
	memcpy(texture->data, stbiData, texture->dataSize);

	stbi_image_free(stbiData);
}

// TODO: signature
// helper functions

static int32_t r_findCompatibleMemoryIndex(
	VkMemoryRequirements memoryRequirements, 
	VkPhysicalDeviceMemoryProperties memoryProperties,
	VkMemoryPropertyFlags memoryPropertyFlags)
{
	// TODO: this should really be a function
	int32_t memoryPropertyIndex = -1;
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if ((memoryRequirements.memoryTypeBits & (1 << i)) && // the fuck?
			(memoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags)
			== memoryPropertyFlags)
		{
			memoryPropertyIndex = (int32_t)i;
			break;
		}
	}
	return memoryPropertyIndex;
}

static VkBuffer r_createBuffer(
	VkDevice device, 
	VkDeviceSize bufferSize, 
	VkBufferUsageFlags usageFlags)
{
	VkBuffer buffer;
	VkResult result;

	VkBufferCreateInfo bufferCreateInfo = { 0 };
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = NULL;
	bufferCreateInfo.flags = 0;
	bufferCreateInfo.size = bufferSize;
	bufferCreateInfo.usage = usageFlags;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	result = vkCreateBuffer(device, &bufferCreateInfo, NULL, &buffer);
	assert(result == VK_SUCCESS);

	return buffer;
}

static VkDeviceMemory r_allocateBufferMemory(
	VkDevice device,
	VkBuffer buffer, 
	VkPhysicalDeviceMemoryProperties memoryProperties,
	VkMemoryPropertyFlags memoryPropertyFlags
	)
{
	VkResult result;

	VkMemoryRequirements bufferMemoryRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &bufferMemoryRequirements);

	int32_t memoryTypeIndex = r_findCompatibleMemoryIndex(
		bufferMemoryRequirements,
		memoryProperties,
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

static VkSemaphore r_createSemaphore(VkDevice device)
{
	VkResult result;
	VkSemaphore semaphore;

	VkSemaphoreCreateInfo semaphoreCreateInfo = { 0 };
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = NULL;
	semaphoreCreateInfo.flags = 0;

	result = vkCreateSemaphore(device, &semaphoreCreateInfo, NULL, &semaphore);
	assert(result == VK_SUCCESS);

	return semaphore;
}

static Vertex *r_createQuad(uint32_t width, uint32_t height)
{
	Vertex *quad = (Vertex*)malloc(6 * sizeof(Vertex));
	// First triangle
	quad[0].pos[0] = -width / 2.0f; // top left
	quad[0].pos[1] = height;

	quad[1].pos[0] = width / 2.0f; // bottom right
	quad[1].pos[1] = 0.0f;

	quad[2].pos[0] = -width / 2.0f; // bottom left
	quad[2].pos[1] = 0.0f;

	// Second triangle
	quad[3].pos[0] = -width / 2.0f; // top left
	quad[3].pos[1] = height;

	quad[4].pos[0] = width / 2.0f; // top right
	quad[4].pos[1] = height;

	quad[5].pos[0] = width / 2.0f; // bottom right
	quad[5].pos[1] = 0.0f;

	return quad;
}

// end

static void r_createTexture()
{
	Texture texture = { 0 };
	r_readImageFile("../resources/doggo.jpg", &texture);

	// TODO: buffer creation helper function

	VkResult result;

	VkDeviceSize stagingBufferSize = texture.dataSize;

	VkBufferCreateInfo stagingBufferCreateInfo = { 0 };
	stagingBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	stagingBufferCreateInfo.pNext = NULL;
	stagingBufferCreateInfo.flags = 0;
	stagingBufferCreateInfo.size = stagingBufferSize;
	stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stagingBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkBuffer stagingBuffer;
	result = vkCreateBuffer(g_deviceInfo.device, &stagingBufferCreateInfo, NULL, &stagingBuffer);
	assert(result == VK_SUCCESS);

	VkMemoryRequirements stagingBufferMemoryRequirements;
	vkGetBufferMemoryRequirements(g_deviceInfo.device, stagingBuffer, &stagingBufferMemoryRequirements);

	// TODO: this should really be a function
	int32_t memoryPropertyIndex = -1;
	for (uint32_t i = 0; i < g_gpuInfo.memoryProperties.memoryTypeCount; i++)
	{
		if ((stagingBufferMemoryRequirements.memoryTypeBits & (1 << i)) && // the fuck?
			(g_gpuInfo.memoryProperties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
			== (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
		{
			memoryPropertyIndex = (int32_t)i;
		}
	}

	assert(memoryPropertyIndex >= 0);

	VkMemoryAllocateInfo memoryAllocateInfo = { 0 };
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = NULL;
	memoryAllocateInfo.allocationSize = stagingBufferMemoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = memoryPropertyIndex;

	VkDeviceMemory stagingBufferMemory;
	result = vkAllocateMemory(g_deviceInfo.device, &memoryAllocateInfo, NULL, &stagingBufferMemory);
	assert(result == VK_SUCCESS);

	result = vkBindBufferMemory(g_deviceInfo.device, stagingBuffer, stagingBufferMemory, 0);
	assert(result == VK_SUCCESS);

	void *data;
	result = vkMapMemory(g_deviceInfo.device, stagingBufferMemory, 0, stagingBufferSize, 0, &data);
	assert(result == VK_SUCCESS);

	memcpy(data, texture.data, texture.dataSize);
	vkUnmapMemory(g_deviceInfo.device, stagingBufferMemory);

	// TODO: maybe flush?

	VkImageCreateInfo imageCreateInfo = { 0 };
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext = NULL;
	imageCreateInfo.flags = 0;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageCreateInfo.extent.width = texture.width;
	imageCreateInfo.extent.height = texture.height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED; // preinitialized = preserve texels, undefined = discard texels

	result = vkCreateImage(g_deviceInfo.device, &imageCreateInfo, NULL, &texture.image);
	assert(result == VK_SUCCESS);

	VkMemoryRequirements imageMemoryRequirements;
	vkGetImageMemoryRequirements(g_deviceInfo.device, texture.image, &imageMemoryRequirements);

	memoryPropertyIndex = -1;
	for (uint32_t i = 0; i < g_gpuInfo.memoryProperties.memoryTypeCount; i++)
	{
		if ((imageMemoryRequirements.memoryTypeBits & (1 << i)) && // the fuck?
			(g_gpuInfo.memoryProperties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
			== (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
		{
			memoryPropertyIndex = (int32_t)i;
		}
	}

	assert(memoryPropertyIndex >= 0);

	VkMemoryAllocateInfo imageMemoryAllocateInfo = { 0 };
	imageMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	imageMemoryAllocateInfo.pNext = NULL;
	imageMemoryAllocateInfo.allocationSize = imageMemoryRequirements.size;
	imageMemoryAllocateInfo.memoryTypeIndex = memoryPropertyIndex;

	result = vkAllocateMemory(g_deviceInfo.device, &imageMemoryAllocateInfo, NULL, &texture.imageMemory);
	assert(result == VK_SUCCESS);

	vkBindImageMemory(g_deviceInfo.device, texture.image, texture.imageMemory, 0);

	// one off
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = { 0 };
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = NULL;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandPool = g_presentCommandPoolInfo.commandPool;
	commandBufferAllocateInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	result = vkAllocateCommandBuffers(g_deviceInfo.device, &commandBufferAllocateInfo, &commandBuffer);
	assert(result == VK_SUCCESS);

	VkCommandBufferBeginInfo commandBufferBeginInfo = { 0 };
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = NULL;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	commandBufferBeginInfo.pInheritanceInfo = NULL;

	VkImageMemoryBarrier imageMemoryBarrier = { 0 };
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.pNext = NULL;
	imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.image = texture.image;
	imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarrier.subresourceRange.levelCount = 1;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarrier.subresourceRange.layerCount = 1;

	VkBufferImageCopy bufferImageCopy = { 0 };
	bufferImageCopy.bufferOffset = 0;
	bufferImageCopy.bufferRowLength = 0;
	bufferImageCopy.bufferImageHeight = 0;
	bufferImageCopy.imageExtent.width = texture.width;
	bufferImageCopy.imageExtent.height = texture.height;
	bufferImageCopy.imageExtent.depth = 1;
	bufferImageCopy.imageOffset.x = 0;
	bufferImageCopy.imageOffset.y = 0;
	bufferImageCopy.imageOffset.z = 0;
	bufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	bufferImageCopy.imageSubresource.mipLevel = 0;
	bufferImageCopy.imageSubresource.baseArrayLayer = 0;
	bufferImageCopy.imageSubresource.layerCount = 1;

	result = vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
	assert(result == VK_SUCCESS);

	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		0,
		0, NULL,
		0, NULL,
		1, &imageMemoryBarrier);
	vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);

	result = vkEndCommandBuffer(commandBuffer);
	assert(result == VK_SUCCESS);

	vkDestroyBuffer(g_deviceInfo.device, stagingBuffer, NULL);
	vkFreeMemory(g_deviceInfo.device, stagingBufferMemory, NULL);

	VkImageViewCreateInfo imageViewCreateInfo = { 0 };
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.pNext = NULL;
	imageViewCreateInfo.flags = 0;
	imageViewCreateInfo.image = texture.image;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;

	result = vkCreateImageView(g_deviceInfo.device, &imageViewCreateInfo, NULL, &texture.imageView);
	assert(result == VK_SUCCESS);

	VkSamplerCreateInfo samplerCreateInfo = { 0 };
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.pNext = NULL;
	samplerCreateInfo.flags = 0;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
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

	result = vkCreateSampler(g_deviceInfo.device, &samplerCreateInfo, NULL, &texture.sampler);
	assert(result == VK_SUCCESS);
}

static void r_readShaders()
{
	FILE *vertexShader;
	assert(fopen_s(&vertexShader, "../vulkangame/vert.spv", "rb") != EINVAL);

	fseek(vertexShader, 0, SEEK_END);
	size_t vertexShaderFileSize = ftell(vertexShader);
	fseek(vertexShader, 0, SEEK_SET);

	g_shaders.vertexShaderFileSize = vertexShaderFileSize;
	g_shaders.vertexShader = (char*)malloc(vertexShaderFileSize + 1);

	fread_s(g_shaders.vertexShader, vertexShaderFileSize + 1, 1, vertexShaderFileSize, vertexShader);

	fclose(vertexShader);

	FILE *fragShader;
	assert(fopen_s(&fragShader, "../vulkangame/frag.spv", "rb") != EINVAL);

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

static void r_createDescriptorSetLayout()
{
	VkResult result;

	VkDescriptorSetLayoutBinding uniformBufferLayoutBinding = { 0 };
	uniformBufferLayoutBinding.binding = 0;
	uniformBufferLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformBufferLayoutBinding.descriptorCount = 1;
	uniformBufferLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uniformBufferLayoutBinding.pImmutableSamplers = NULL;

	VkDescriptorSetLayoutBinding samplerLayoutBinding = { 0 };
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	samplerLayoutBinding.pImmutableSamplers = NULL;

	VkDescriptorSetLayoutBinding descriptorSetLayoutBindings[] =
	{
		uniformBufferLayoutBinding,
		samplerLayoutBinding
	};

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = { 0 };
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.pNext = NULL;
	descriptorSetLayoutCreateInfo.flags = 0;
	descriptorSetLayoutCreateInfo.bindingCount = 2;
	descriptorSetLayoutCreateInfo.pBindings = &descriptorSetLayoutBindings;

	result = vkCreateDescriptorSetLayout(g_deviceInfo.device, &descriptorSetLayoutCreateInfo, NULL, &g_descriptorInfo.setLayout);
	assert(result == VK_SUCCESS);
}

static void r_createDescriptorPool()
{
	VkResult result;

	VkDescriptorPoolSize descriptorPoolSizes[] = {
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 }
	};

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = { 0 };
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.pNext = NULL;
	descriptorPoolCreateInfo.flags = 0;
	descriptorPoolCreateInfo.poolSizeCount = 2;
	descriptorPoolCreateInfo.pPoolSizes = &descriptorPoolSizes;
	descriptorPoolCreateInfo.maxSets = 1;

	result = vkCreateDescriptorPool(g_deviceInfo.device, &descriptorPoolCreateInfo, NULL, &g_descriptorInfo.pool);
	assert(result == VK_SUCCESS);
}

static void r_createDescriptorSet()
{
	VkResult result;

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = { 0 };
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.pNext = NULL;
	descriptorSetAllocateInfo.descriptorPool = g_descriptorInfo.pool;
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	descriptorSetAllocateInfo.pSetLayouts = &g_descriptorInfo.setLayout;

	result = vkAllocateDescriptorSets(g_deviceInfo.device, &descriptorSetAllocateInfo, &g_descriptorInfo.set);
	assert(result == VK_SUCCESS);

	VkDescriptorBufferInfo descriptorBufferInfo = { 0 };
	descriptorBufferInfo.buffer = g_uniformBuffer.buffer;
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range = sizeof(Matrix4x4);

	VkWriteDescriptorSet writeDescriptorSet = { 0 };
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.dstSet = g_descriptorInfo.set;
	writeDescriptorSet.dstBinding = 0;
	writeDescriptorSet.dstArrayElement = 0;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;
	writeDescriptorSet.pImageInfo = NULL;
	writeDescriptorSet.pTexelBufferView = NULL;

	vkUpdateDescriptorSets(g_deviceInfo.device, 1, &writeDescriptorSet, 0, NULL);
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

	VkVertexInputBindingDescription vertexInputBindingDescription = r_createVertexBindingDescription(0);

	VkVertexInputAttributeDescription vertexInputAttributeDescriptions[2] =
	{
		r_createPositionVertexAttributeDescription(0, 0),
		r_createColorVertexAttributeDescription(0, 1)
	};

	VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = { 0 };
	pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	pipelineVertexInputStateCreateInfo.pNext = NULL;
	pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
	pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
	pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = 2;
	pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescriptions;

	VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo = { 0 };
	pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pipelineInputAssemblyStateCreateInfo.pNext = NULL;
	pipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = { 0 };
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = g_presentInfo.imagesExtent.width;
	viewport.height = g_presentInfo.imagesExtent.height;
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
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &g_descriptorInfo.setLayout;
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

	VkPipelineStageFlags pipelineStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submitInfo = { 0 };
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = NULL;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &g_presentInfo.semaphore;
	submitInfo.pWaitDstStageMask = &pipelineStageFlags;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &g_presentCommandPoolInfo.commandBuffers[g_swapchainInfo.nextImageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &g_presentInfo.semaphore;

	vkQueueSubmit(g_deviceInfo.presentQueue, 1, &submitInfo, VK_NULL_HANDLE);

	VkPresentInfoKHR presentInfo = { 0 };
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = NULL;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &g_presentInfo.semaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &g_swapchainInfo.swapchain;
	presentInfo.pImageIndices = &g_swapchainInfo.nextImageIndex;
	presentInfo.pResults = NULL;

	vkQueuePresentKHR(g_deviceInfo.presentQueue, &presentInfo);
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
		1280, // width
		720, // height
		NULL,
		NULL,
		hInstance,
		NULL
	);

	return window;
}