#define VK_USE_PLATFORM_WIN32_KHR

#include <Windows.h>
#include <assert.h>
#include <tchar.h>
#include "vulkan\vulkan.h"
#include "datatest.h"

// TODO: Ensure a compatible card is chosen.
// TODO: Remove magic numbers.
// TODO: Consider queues.
// TODO: Clean up.
// TODO: Non-resizable window.
// TODO: CopyFile to avoid dll locking

VkInstance createVkInstance();
VkPhysicalDevice getVkPhysicalDevice(VkInstance instance);
VkSurfaceKHR createVkSurface(VkInstance instance, HINSTANCE hInstance, HWND window);

void dllTest()
{
	DataTest test = { 0 };
	HMODULE test_dll = LoadLibrary(L"test.dll");
	void(*func)(DataTest*) = (void*)GetProcAddress(test_dll, "testprint");
	func(&test);
	FreeLibrary(test_dll);
	int i = 0;
}

// TODO: clean up
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		TextOut(hdc, 5, 5, _T("hej"), _tcslen(_T("hej")));
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}

	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	// Vulkan block

	// TODO: Ensure a compatible device is chosen.
	// TODO: Ensure queue supports presentation.
	// TODO: Ensure device supports swapchain (VK_KHR_SWAPCHAIN_EXTENSION_NAME).
	// TODO: Remove magic numbers.
	// TODO: Consider queues.
	// TODO: Clean up.
	// TODO: Non-resizable window.

	VkResult result;

	VkInstance instance = createVkInstance();
	VkPhysicalDevice physical_device = getVkPhysicalDevice(instance);

	// TODO: create function
	uint32_t queue_families_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_families_count, NULL);
	assert(queue_families_count > 0);

	VkQueueFamilyProperties *queue_families =
		(VkQueueFamilyProperties*)malloc(queue_families_count * sizeof(VkQueueFamilyProperties)); // TODO: free
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_families_count, queue_families);

	uint32_t queue_family_index = -1;
	VkQueueFlags desired_capabilities = VK_QUEUE_GRAPHICS_BIT;
	for (uint32_t i = 0; i < queue_families_count; i++)
	{
		if (queue_families[i].queueCount > 0 && (queue_families[i].queueFlags & desired_capabilities) != 0)
		{
			queue_family_index = i;
			break;
		}
	}
	assert(queue_family_index >= 0);

	float priority = 1.0f;

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
	device_create_info.enabledLayerCount = 0;
	device_create_info.ppEnabledLayerNames = NULL;
	device_create_info.enabledExtensionCount = 1;
	device_create_info.ppEnabledExtensionNames = desired_device_extensions;
	device_create_info.pEnabledFeatures = NULL;

	VkDevice logical_device;
	result = vkCreateDevice(physical_device, &device_create_info, NULL, &logical_device);
	assert(result == VK_SUCCESS);

	VkQueue queue;
	vkGetDeviceQueue(logical_device, queue_family_index, 0, &queue);

	// Window block

	// TODO: clean up
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

	VkSurfaceKHR presentation_surface = createVkSurface(instance, hInstance, window);

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

	VkImageUsageFlags desired_usages = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	VkImageUsageFlags image_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	// TODO: set desired transformation of swapchain images

	VkSurfaceFormatKHR desired_surface_format;
	uint32_t formats_count;
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, presentation_surface, &formats_count, NULL);
	assert(result == VK_SUCCESS);
	
	VkSurfaceFormatKHR *surface_formats = 
		(VkSurfaceFormatKHR*)malloc(formats_count * sizeof(VkSurfaceFormatKHR)); // TODO: free
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, presentation_surface, &formats_count, surface_formats);
	assert(result == VK_SUCCESS);

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

	VkSwapchainKHR old_swapchain = VK_NULL_HANDLE;

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
	swapchain_create_info.oldSwapchain = old_swapchain;

	VkSwapchainKHR swapchain;
	result = vkCreateSwapchainKHR(logical_device, &swapchain_create_info, NULL, &swapchain);
	assert(result == VK_SUCCESS);

	vkDestroySwapchainKHR(logical_device, old_swapchain, NULL);
	
	uint32_t images_count;
	result = vkGetSwapchainImagesKHR(logical_device, swapchain, &images_count, NULL);
	assert(result == VK_SUCCESS);

	VkImage *swapchain_images = 
		(VkImage*)malloc(images_count * sizeof(VkImage)); // TODO: free
	result = vkGetSwapchainImagesKHR(logical_device, swapchain, &images_count, swapchain_images);
	assert(result == VK_SUCCESS);

	VkSemaphoreCreateInfo semaphore_create_info = { 0 };
	semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphore_create_info.pNext = NULL;
	semaphore_create_info.flags = 0;

	VkSemaphore semaphore;
	
	result = vkCreateSemaphore(logical_device, &semaphore_create_info, NULL, &semaphore);
	assert(result == VK_SUCCESS);

	uint32_t image_index;
	result = vkAcquireNextImageKHR(logical_device, swapchain, 10, semaphore, NULL, &image_index);
	assert(result == VK_SUCCESS);

	//VkSwapchainKHR swapchain;

	//VkFormat image_format = VK_FORMAT_R8G8B8A8_UNORM;
	//VkColorSpaceKHR image_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

	ShowWindow(window, nShowCmd);

	//while (1)
	//{
		//dllTest();
	//}


	// TODO: peek message
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
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