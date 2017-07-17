#define VK_USE_PLATFORM_WIN32_KHR

#include <Windows.h>
#include <assert.h>
#include <tchar.h>
#include "vulkaninfo.h"
#include "vulkan\vulkan.h"
#include "datatest.h"
#include "simd_math.h"
#include "renderer.h"


// TODO: Ensure a compatible card is chosen.
// TODO: Remove magic numbers.
// TODO: Consider queues.
// TODO: Clean up.
// TODO: Non-resizable window / stop surface from breaking on resize.
// TODO: CopyFile to avoid dll locking
// TODO: Peek message + proper gameloop.

// Heap:
// Swapchain+images+count
// Queue indices
// Synchronization primitives (memory barriers/semaphores/fences)
// Command pools/buffers
// Presentation surface


VkInstance createVkInstance();
boolean isExtensionAvailable(const char *const desiredExtension, VkExtensionProperties *availableExtensions, uint32_t extensionsCount);
void getInfoForAllGpus(GpuInfo *gpuInfos, VkPhysicalDevice *availableDevices, uint32_t gpuCount);
int32_t getFirstCompatibleGpuIndex(GpuInfo *gpuInfos, uint32_t gpuCount);
uint32_t getVkGraphicsQueueFamilyIndex(VkPhysicalDevice physical_device);
VkPhysicalDevice getVkPhysicalDevice(VkInstance instance);
VkSurfaceKHR createVkSurface(VkInstance instance, HINSTANCE hInstance, HWND window);

//HWND createAndRegisterWindow(HINSTANCE hInstance);

void dllTest()
{
	DataTest test = { 0 };
	HMODULE test_dll = LoadLibrary(L"test.dll");
	void(*func)(DataTest*) = (void*)GetProcAddress(test_dll, "testprint");
	func(&test);
	FreeLibrary(test_dll);
	int i = 0;
}

#define TEST
#ifndef TEST

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




int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	Vector3 vec = {
		25.0f,
		36.0f,
		64.0f
	};
	Vector3 unitTest = convertVectorToUnitLength(vec);
	float lengthTest = getVectorLength(unitTest);

	//r_readShaders();

	// Vulkan block

	// TODO: Ensure a compatible device is chosen.
	// TODO: Ensure queue supports presentation.
	// TODO: Ensure device supports swapchain (VK_KHR_SWAPCHAIN_EXTENSION_NAME).
	// TODO: Remove magic numbers.
	// TODO: Some voodoo if present and graphics are separate queues
	// TODO: Consider queues.
	// TODO: Clean up.
	// TODO: Non-resizable window.

	VkResult result;
	VkInstance instance = createVkInstance();

	r_initVulkan();
	//GpuInfo gpugug = r_test_getGpuInfo();

	VkPhysicalDevice physical_device = getVkPhysicalDevice(instance);
	uint32_t queue_family_index = getVkGraphicsQueueFamilyIndex(physical_device);

	// starting clean up, not used yet ->
	uint32_t gpuCount;
	result = vkEnumeratePhysicalDevices(instance, &gpuCount, NULL);
	assert(result == VK_SUCCESS);

	VkPhysicalDevice *availableDevices =
		(VkPhysicalDevice*)malloc(gpuCount * sizeof(VkPhysicalDevice)); // TODO: free
	result = vkEnumeratePhysicalDevices(instance, &gpuCount, availableDevices);

	GpuInfo *gpuInfos =
		(GpuInfo*)malloc(gpuCount * sizeof(GpuInfo)); // TODO: free

	getInfoForAllGpus(gpuInfos, availableDevices, gpuCount);

	int32_t gpuIndex = getFirstCompatibleGpuIndex(gpuInfos, gpuCount);
	assert(gpuIndex >= 0);

	GpuInfo gpuInfo = gpuInfos[gpuIndex];
	// clean up <-

	// TODO: dunno
	float priority = 1.0f;

	// queue family bits
	// VK_QUEUE_GRAPHICS_BIT = 0x00000001
	// VK_QUEUE_COMPUTE_BIT = 0x00000002
	// VK_QUEUE_TRANSFER_BIT = 0x00000004
	// VK_QUEUE_SPARSE_BINDING_BIT = 0x00000008

	VkDeviceQueueCreateInfo queue_create_info = { 0 };
	queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_create_info.pNext = NULL;
	queue_create_info.flags = 0;
	queue_create_info.queueFamilyIndex = queue_family_index;
	queue_create_info.queueCount = 1;
	queue_create_info.pQueuePriorities = &priority;

	const char *const desired_device_extensions[1] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VkDeviceCreateInfo device_create_info = { 0 };
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.pNext = NULL;
	device_create_info.flags = 0;
	device_create_info.queueCreateInfoCount = 1;
	device_create_info.pQueueCreateInfos = &queue_create_info;
	device_create_info.enabledLayerCount = 0; // deprecated 
	device_create_info.ppEnabledLayerNames = NULL; // deprecated 
	device_create_info.enabledExtensionCount = 1;
	device_create_info.ppEnabledExtensionNames = desired_device_extensions;
	device_create_info.pEnabledFeatures = NULL;

	VkDevice logical_device;
	result = vkCreateDevice(physical_device, &device_create_info, NULL, &logical_device);
	assert(result == VK_SUCCESS);

	VkQueue queue;
	vkGetDeviceQueue(logical_device, queue_family_index, 0, &queue);

	// command buffer

	VkCommandPoolCreateInfo command_pool_create_info = { 0 };
	command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_create_info.pNext = NULL;
	command_pool_create_info.flags = 0;
	command_pool_create_info.queueFamilyIndex = queue_family_index;

	VkCommandPool command_pool; // 1 thread per pool
	result = vkCreateCommandPool(logical_device, &command_pool_create_info, NULL, &command_pool);
	assert(result == VK_SUCCESS);

	VkCommandBufferAllocateInfo command_buffer_allocate_info = { 0 };
	command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_allocate_info.pNext = NULL;
	command_buffer_allocate_info.commandPool = command_pool;
	command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_allocate_info.commandBufferCount = 1;

	VkCommandBuffer command_buffer;
	result = vkAllocateCommandBuffers(logical_device, &command_buffer_allocate_info, &command_buffer);
	assert(result == VK_SUCCESS);

	
	HWND window = createAndRegisterWindow(hInstance);

	// TODO: vkGetPhysicalDeviceSurfaceSupportKHR()
	VkSurfaceKHR presentation_surface = createVkSurface(instance, hInstance, window);

	VkBool32 surface_supported;
	result = vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, queue_family_index, presentation_surface, &surface_supported);
	assert(result == VK_SUCCESS);
	assert(surface_supported == VK_TRUE);

	// TODO: Consider presentation modes, and check if desired is available.
	// TODO: Create function.
	VkPresentModeKHR desired_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
	uint32_t present_modes_count = 0;
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, presentation_surface, &present_modes_count, NULL);
	assert(result == VK_SUCCESS);

	VkPresentModeKHR *present_modes =
		(VkPresentModeKHR*)malloc(present_modes_count * sizeof(VkPresentModeKHR)); // TODO: free
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, presentation_surface, &present_modes_count, present_modes);
	assert(result == VK_SUCCESS);

	VkSurfaceCapabilitiesKHR surface_capabilities;
	result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, presentation_surface, &surface_capabilities);
	assert(result == VK_SUCCESS);

	uint32_t number_of_images = surface_capabilities.minImageCount + 1;
	if (surface_capabilities.maxImageCount > 0)
	{
		if (number_of_images > surface_capabilities.maxImageCount)
			number_of_images = surface_capabilities.maxImageCount;
	}

	VkExtent2D size_of_images;

	if ((uint32_t)surface_capabilities.currentExtent.width == -1)
	{
		// TODO: implement case
	}
	else
	{
		// size_of_images = surface_capabilities.currentExtent;
	}

	size_of_images = surface_capabilities.currentExtent;

	// TODO: check desired/supported usages.

	VkImageUsageFlags desired_usages = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	VkImageUsageFlags image_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	// TODO: set desired transformation of swapchain images

	// TODO: do something with this
	VkSurfaceFormatKHR desired_surface_format;
	uint32_t formats_count;
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, presentation_surface, &formats_count, NULL);
	assert(result == VK_SUCCESS);

	VkSurfaceFormatKHR *surface_formats =
		(VkSurfaceFormatKHR*)malloc(formats_count * sizeof(VkSurfaceFormatKHR)); // TODO: free
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, presentation_surface, &formats_count, surface_formats);
	assert(result == VK_SUCCESS);

	// TODO: Check supported formats
	VkFormat image_format = VK_FORMAT_B8G8R8A8_UNORM;
	VkColorSpaceKHR image_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

	if (formats_count == 1 &&
		surface_formats->format == VK_FORMAT_UNDEFINED)
	{
		// TODO: implement case
	}
	else
	{
		// TODO: implement case
	}

	VkSwapchainCreateInfoKHR swapchain_create_info = { 0 };
	swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_create_info.pNext = NULL;
	swapchain_create_info.flags = 0;
	swapchain_create_info.surface = presentation_surface;
	swapchain_create_info.minImageCount = number_of_images;
	swapchain_create_info.imageFormat = image_format;
	swapchain_create_info.imageColorSpace = image_color_space;
	swapchain_create_info.imageExtent = size_of_images;
	swapchain_create_info.imageArrayLayers = 1;
	swapchain_create_info.imageUsage = desired_usages;
	swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchain_create_info.queueFamilyIndexCount = 0;
	swapchain_create_info.pQueueFamilyIndices = NULL;
	swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_create_info.presentMode = *present_modes;
	swapchain_create_info.clipped = VK_TRUE;
	swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

	VkSwapchainKHR swapchain;
	result = vkCreateSwapchainKHR(logical_device, &swapchain_create_info, NULL, &swapchain);
	assert(result == VK_SUCCESS);

	uint32_t images_count;
	result = vkGetSwapchainImagesKHR(logical_device, swapchain, &images_count, NULL);
	assert(result == VK_SUCCESS);

	VkImage *swapchain_images =
		(VkImage*)malloc(images_count * sizeof(VkImage)); // TODO: free
	result = vkGetSwapchainImagesKHR(logical_device, swapchain, &images_count, swapchain_images); // List of handles to images
	assert(result == VK_SUCCESS);


	// bind memory objects to swapchain images
	VkMemoryRequirements *memoryRequirements =
		(VkMemoryRequirements*)malloc(images_count * sizeof(VkMemoryRequirements)); // TODO: free
	for (uint32_t i = 0; i < images_count; i++)
	{
		vkGetImageMemoryRequirements(logical_device, swapchain_images[0], &memoryRequirements[i]);
	}

	VkDeviceMemory *memoryObjects =
		(VkDeviceMemory*)malloc(images_count * sizeof(VkDeviceMemory)); // TODO: free
	for (uint32_t i = 0; i < images_count; i++)
	{
		memoryObjects[i] = VK_NULL_HANDLE;
	}

	for (uint32_t i = 0; i < images_count; i++)
	{
		for (uint32_t j = 1; j < gpuInfo.memoryProperties.memoryTypeCount; j++)
		{
			if ((memoryRequirements[i].memoryTypeBits & (1 << j)))
			{
				VkMemoryAllocateInfo imageMemoryAllocateInfo = { 0 };
				imageMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				imageMemoryAllocateInfo.pNext = NULL;
				imageMemoryAllocateInfo.allocationSize = memoryRequirements[0].size;
				imageMemoryAllocateInfo.memoryTypeIndex = j;

				result = vkAllocateMemory(logical_device, &imageMemoryAllocateInfo, NULL, &memoryObjects[i]);
				assert(result == VK_SUCCESS);
				break;
			}
		}
	}

	VkImageMemoryBarrier imageMemoryBarrier = { 0 };
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.pNext = NULL;
	imageMemoryBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	imageMemoryBarrier.srcQueueFamilyIndex = queue_family_index;
	imageMemoryBarrier.dstQueueFamilyIndex = queue_family_index;
	imageMemoryBarrier.image = swapchain_images[0];
	imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarrier.subresourceRange.levelCount = 1;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarrier.subresourceRange.layerCount = 1;

	// swapchain image views
	VkImageSubresourceRange subresourceRange = { 0 };
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = 1;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.layerCount = 1;

	VkImageViewCreateInfo *image_view_create_infos =
		(VkImageViewCreateInfo*)malloc(images_count * sizeof(VkImageViewCreateInfo)); // TODO: free
	VkImageView *image_views =
		(VkImageView*)malloc(images_count * sizeof(VkImageView));
	for (uint32_t i = 0; i < images_count; i++)
	{
		image_view_create_infos[i].sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_create_infos[i].pNext = NULL;
		image_view_create_infos[i].flags = 0;
		image_view_create_infos[i].image = swapchain_images[i];
		image_view_create_infos[i].viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_create_infos[i].format = image_format;
		image_view_create_infos[i].components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_infos[i].components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_infos[i].components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_infos[i].components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_infos[i].subresourceRange = subresourceRange;

		result = vkCreateImageView(logical_device, &image_view_create_infos[i], NULL, &image_views[i]);
		assert(result == VK_SUCCESS);
	}

	VkSemaphoreCreateInfo semaphore_create_info = { 0 };
	semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphore_create_info.pNext = NULL;
	semaphore_create_info.flags = 0;

	VkSemaphore semaphore;

	result = vkCreateSemaphore(logical_device, &semaphore_create_info, NULL, &semaphore);
	assert(result == VK_SUCCESS);

	uint32_t image_index;
	result = vkAcquireNextImageKHR(logical_device, swapchain, 2000000000, semaphore, NULL, &image_index); // index in list of handles
	assert(result == VK_SUCCESS);
	// VK_SUBOPTIMAL_KHR - recreate
	// VK_ERROR_OUT_OF_DATE - recreate

	//uint32_t image_indices = 1;

	//

	// CLEAR 

	VkImageMemoryBarrier imageMemoryBarrierUndefinedToTransfer = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,     // VkStructureType                        sType
		NULL,                                    // const void                            *pNext
		0,                  // VkAccessFlags                          srcAccessMask
		VK_ACCESS_TRANSFER_WRITE_BIT,               // VkAccessFlags                          dstAccessMask
		VK_IMAGE_LAYOUT_UNDEFINED,                  // VkImageLayout                          oldLayout
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,       // VkImageLayout                          newLayout
		VK_QUEUE_FAMILY_IGNORED,                    // uint32_t                               srcQueueFamilyIndex
		VK_QUEUE_FAMILY_IGNORED,                    // uint32_t                               dstQueueFamilyIndex
		swapchain_images[image_index],                       // VkImage                                image
		subresourceRange                    // VkImageSubresourceRange                subresourceRange
	};

	VkImageMemoryBarrier imageMemoryBarrierTransferToPresent = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,     // VkStructureType                        sType
		NULL,                                    // const void                            *pNext
		VK_ACCESS_TRANSFER_WRITE_BIT,                  // VkAccessFlags                          srcAccessMask
		VK_ACCESS_TRANSFER_WRITE_BIT,               // VkAccessFlags                          dstAccessMask
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,                  // VkImageLayout                          oldLayout
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,       // VkImageLayout                          newLayout
		VK_QUEUE_FAMILY_IGNORED,                    // uint32_t                               srcQueueFamilyIndex
		VK_QUEUE_FAMILY_IGNORED,                    // uint32_t                               dstQueueFamilyIndex
		swapchain_images[image_index],                       // VkImage                                image
		subresourceRange                    // VkImageSubresourceRange                subresourceRange
	};

	VkCommandBufferBeginInfo commandBufferBeginInfo = { 0 };
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = NULL;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	commandBufferBeginInfo.pInheritanceInfo = NULL;

	VkClearColorValue clear_color = {
		{ 0.5f, 0.5f, 1.0f, 0.0f }
	};

	result = vkBeginCommandBuffer(command_buffer, &commandBufferBeginInfo);
	vkCmdPipelineBarrier(command_buffer, 
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 
		0, 0, NULL, 0, NULL, 1, &imageMemoryBarrierUndefinedToTransfer);
	vkCmdClearColorImage(command_buffer, swapchain_images[image_index],
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_color, 
		1, &subresourceRange);
	vkCmdPipelineBarrier(command_buffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		0, 0, NULL, 0, NULL, 1, &imageMemoryBarrierTransferToPresent);
	result = vkEndCommandBuffer(command_buffer);

	VkSubmitInfo submitInfo = { 0 };
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = NULL;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = NULL;
	submitInfo.pWaitDstStageMask = NULL;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &command_buffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = NULL;

	result = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	// VkQueueSubmit

	//

	VkPresentInfoKHR present_info = { 0 };
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.pNext = NULL;
	present_info.waitSemaphoreCount = 0;
	present_info.pWaitSemaphores = NULL;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &swapchain;
	present_info.pImageIndices = &image_index;
	
	ShowWindow(window, nShowCmd);	
	
	result = vkQueuePresentKHR(queue, &present_info);

	// depth buffer
	VkImageCreateInfo depthBufferCreateInfo = {0};
	depthBufferCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	depthBufferCreateInfo.pNext = NULL;
	depthBufferCreateInfo.flags = 0;
	depthBufferCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	depthBufferCreateInfo.format = VK_FORMAT_D16_UNORM;
	depthBufferCreateInfo.extent.width = size_of_images.width;
	depthBufferCreateInfo.extent.height = size_of_images.height;
	depthBufferCreateInfo.extent.depth = 1;
	depthBufferCreateInfo.mipLevels = 1;
	depthBufferCreateInfo.arrayLayers = 1;
	depthBufferCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	depthBufferCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthBufferCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	depthBufferCreateInfo.queueFamilyIndexCount = 0;
	depthBufferCreateInfo.pQueueFamilyIndices = NULL;
	depthBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkImage depthBuffer;
	result = vkCreateImage(logical_device, &depthBufferCreateInfo, NULL, &depthBuffer);
	assert(result == VK_SUCCESS);

	VkMemoryRequirements depthBufferMemoryRequirements;
	vkGetImageMemoryRequirements(logical_device, depthBuffer, &depthBufferMemoryRequirements);
	int32_t memoryIndex = 8; // TODO: oh gosh

	//for (uint32_t i = 0; i < gpuInfo.memoryProperties.memoryTypeCount; i++)
	//{
	//	
	//}
	//assert(memoryIndex >= 0);

	VkMemoryAllocateInfo depthBufferMemoryAllocationInfo = { 0 };
	depthBufferMemoryAllocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	depthBufferMemoryAllocationInfo.pNext = NULL;
	depthBufferMemoryAllocationInfo.allocationSize = depthBufferMemoryRequirements.size;
	depthBufferMemoryAllocationInfo.memoryTypeIndex = memoryIndex;

	VkDeviceMemory depthBufferMemory;
	result = vkAllocateMemory(logical_device, &depthBufferMemoryAllocationInfo, NULL, &depthBufferMemory);
	assert(result == VK_SUCCESS);

	//

	result = vkBindImageMemory(logical_device, depthBuffer, depthBufferMemory, 0);
	assert(result == VK_SUCCESS);

	VkImageViewCreateInfo depthBufferViewCreateInfo = { 0 };
	depthBufferViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	depthBufferViewCreateInfo.pNext = NULL;
	depthBufferViewCreateInfo.flags = 0;
	depthBufferViewCreateInfo.image = depthBuffer;
	depthBufferViewCreateInfo.format = VK_FORMAT_D16_UNORM;
	depthBufferViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	depthBufferViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	depthBufferViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	depthBufferViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	depthBufferViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	depthBufferViewCreateInfo.subresourceRange.baseMipLevel = 0;
	depthBufferViewCreateInfo.subresourceRange.levelCount = 1;
	depthBufferViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	depthBufferViewCreateInfo.subresourceRange.layerCount = 1;
	depthBufferViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	
	VkImageView depthBufferView;
	result = vkCreateImageView(logical_device, &depthBufferViewCreateInfo, NULL, &depthBufferView);
	assert(result == VK_SUCCESS);

	// Uniform buffer
	// TODO: Matrix transformations

	//

	// Descriptor set layouts
	VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = { 0 };
	descriptorSetLayoutBinding.binding = 0;
	descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorSetLayoutBinding.descriptorCount = 1;
	descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	descriptorSetLayoutBinding.pImmutableSamplers = NULL;

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = { 0 };
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.pNext = NULL;
	descriptorSetLayoutCreateInfo.bindingCount = 1;
	descriptorSetLayoutCreateInfo.pBindings = &descriptorSetLayoutBinding;

	VkDescriptorSetLayout descriptorSetLayout;
	result = vkCreateDescriptorSetLayout(logical_device, &descriptorSetLayoutCreateInfo, NULL, &descriptorSetLayout);
	assert(result == VK_SUCCESS);

	// Pipeline layouts
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = { 0 };
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pNext = NULL;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = NULL;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;

	VkPipelineLayout pipelineLayout;
	result = vkCreatePipelineLayout(logical_device, &pipelineLayoutCreateInfo, NULL, &pipelineLayout);
	assert(result == VK_SUCCESS);

	// Descriptor pools
	VkDescriptorPoolSize descriptorPoolSize = { 0 };
	descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorPoolSize.descriptorCount = 1;

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = { 0 };
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.pNext = NULL;
	descriptorPoolCreateInfo.maxSets = 1;
	descriptorPoolCreateInfo.poolSizeCount = 1;
	descriptorPoolCreateInfo.pPoolSizes = &descriptorPoolSize;

	VkDescriptorPool descriptorPool;
	result = vkCreateDescriptorPool(logical_device, &descriptorPoolCreateInfo, NULL, &descriptorPool);
	assert(result == VK_SUCCESS);

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = { 0 };
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.pNext = NULL;
	descriptorSetAllocateInfo.descriptorPool = descriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;
	
	VkDescriptorSet descriptorSet;
	result = vkAllocateDescriptorSets(logical_device, &descriptorSetAllocateInfo, &descriptorSet);
	assert(result == VK_SUCCESS);

	VkWriteDescriptorSet writeDescriptorSet = { 0 };
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.pNext = NULL;
	writeDescriptorSet.dstSet = descriptorSet;
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	/* 
	writeDescriptorSet.pBufferInfo = pekare till buffern
	writeDescriptorSet.dstArrayElement
	writeDescriptorSet.dstBinding
	*/

	// TODO: peek message
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

#else

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	OsParams params = { 0 };
	params.hInstance = hInstance;
	params.hPrevInstance = hPrevInstance;
	params.lpCmdLine = lpCmdLine;
	params.nShowCmd = nShowCmd;

	r_initVulkan(params);
	r_renderFrame();

	PresentInfo prsntinfo = r_test_getPresentInfo();
	SwapchainInfo swapchaininfo = r_test_getSwapchainInfo();

	r_showWindow();
	r_handleOSMessages();

	return 0;
}

#endif // 

boolean isExtensionAvailable(const char *const desiredExtension, VkExtensionProperties *availableExtensions, uint32_t extensionsCount)
{
	for (uint32_t i = 0; i < extensionsCount; i++)
	{
		if (strcmp(desiredExtension, availableExtensions[i].extensionName) == 0)
			return TRUE;
	}
	return FALSE;
}

VkInstance createVkInstance()
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

void getInfoForAllGpus(GpuInfo *gpuInfos, VkPhysicalDevice *availableDevices, uint32_t gpuCount)
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

int32_t getFirstCompatibleGpuIndex(GpuInfo *gpuInfos, uint32_t gpuCount)
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
			if (!isExtensionAvailable(desiredExtensions[j], gpuInfos[i].extensionProperties, desiredExtensionsCount))
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

uint32_t getVkGraphicsQueueFamilyIndex(VkPhysicalDevice physical_device)
{
	uint32_t queue_families_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_families_count, NULL);
	assert(queue_families_count > 0);

	VkQueueFamilyProperties *queue_families =
		(VkQueueFamilyProperties*)malloc(queue_families_count * sizeof(VkQueueFamilyProperties)); // TODO: free
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_families_count, queue_families);

	uint32_t queue_family_index = queue_families_count;
	VkQueueFlags desired_capabilities = VK_QUEUE_GRAPHICS_BIT;
	for (uint32_t i = 0; i < queue_families_count; i++)
	{
		if (queue_families[i].queueCount > 0 && (queue_families[i].queueFlags & desired_capabilities) != 0)
		{
			queue_family_index = i;
			break;
		}
	}
	assert(queue_family_index != queue_families_count);

	free(queue_families);

	return queue_family_index;
}

// TODO: Ensure a compatible device is chosen (extensions, features, properties).
VkPhysicalDevice getVkPhysicalDevice(VkInstance instance)
{
	VkResult result;

	uint32_t devices_count;
	result = vkEnumeratePhysicalDevices(instance, &devices_count, NULL);
	assert(result == VK_SUCCESS);

	VkPhysicalDevice *available_devices =
		(VkPhysicalDevice*)malloc(devices_count * sizeof(VkPhysicalDevice)); // TODO: free
	result = vkEnumeratePhysicalDevices(instance, &devices_count, available_devices);
	assert(result == VK_SUCCESS);

	VkPhysicalDevice physical_device = available_devices[0];

	// TODO: Ensure device supports swapchain
	//uint32_t device_extensions_count;
	//result = vkEnumerateDeviceExtensionProperties(physical_device, NULL, &device_extensions_count, NULL);
	//assert(result == VK_SUCCESS);

	//
	//VkExtensionProperties *available_device_extensions =
	//	(VkExtensionProperties*)malloc(device_extensions_count * sizeof(VkExtensionProperties)); // TODO: free
	//result = vkEnumerateDeviceExtensionProperties(physical_device, NULL,
	//	&device_extensions_count, available_device_extensions);
	//assert(result == VK_SUCCESS);

	//VkPhysicalDeviceFeatures device_features;
	//vkGetPhysicalDeviceFeatures(physical_device, &device_features);

	//VkPhysicalDeviceProperties device_properties;
	//vkGetPhysicalDeviceProperties(physical_device, &device_properties);

	free(available_devices);

	return physical_device;
}

VkSurfaceKHR createVkSurface(VkInstance instance, HINSTANCE hInstance, HWND window)
{
	VkResult result;

	VkWin32SurfaceCreateInfoKHR surface_create_info = { 0 };
	surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surface_create_info.pNext = NULL;
	surface_create_info.flags = 0;
	surface_create_info.hinstance = hInstance;
	surface_create_info.hwnd = window;

	VkSurfaceKHR presentation_surface = VK_NULL_HANDLE;
	result = vkCreateWin32SurfaceKHR(instance, &surface_create_info, NULL, &presentation_surface);
	assert(result == VK_SUCCESS);
	assert(presentation_surface != VK_NULL_HANDLE);

	return presentation_surface;
}

// TODO: clean up
#ifndef TEST

HWND createAndRegisterWindow(HINSTANCE hInstance)
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

#endif // !TEST