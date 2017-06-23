#include <Windows.h>
#include <assert.h>
#include <tchar.h>
#include "vulkan\vulkan.h"
#include "datatest.h"

// TODO: expand
// TODO: CopyFile to avoid dll locking
void dllTest()
{
	DataTest test = {0};
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
	// -> TODO: Ensure a compatible card is chosen.
	// -> TODO: Remove magic numbers.
	// -> TODO: Consider dynamic linkage.
	// -> TODO: Architecture?
	// -> TODO: Clean up.

	VkResult result;

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
	instance_create_info.enabledExtensionCount = 0;
	instance_create_info.ppEnabledExtensionNames = NULL;

	VkInstance instance;
	result = vkCreateInstance(&instance_create_info, NULL, &instance);
	assert(result == VK_SUCCESS);

	uint32_t devices_count;
	result = vkEnumeratePhysicalDevices(instance, &devices_count, NULL);
	assert(result == VK_SUCCESS);

	VkPhysicalDevice *available_devices =
		(VkPhysicalDevice*)malloc(devices_count * sizeof(VkPhysicalDevice)); // TODO: free
	result = vkEnumeratePhysicalDevices(instance, &devices_count, available_devices);
	assert(result == VK_SUCCESS);

	VkPhysicalDevice physical_device = available_devices[0];

	uint32_t device_extensions_count;
	result = vkEnumerateDeviceExtensionProperties(physical_device, NULL, &device_extensions_count, NULL);
	assert(result == VK_SUCCESS);

	VkExtensionProperties *available_device_extensions =
		(VkExtensionProperties*)malloc(device_extensions_count * sizeof(VkExtensionProperties)); // TODO: free
	result = vkEnumerateDeviceExtensionProperties(physical_device, NULL,
		&device_extensions_count, available_device_extensions);
	assert(result == VK_SUCCESS);

	VkPhysicalDeviceFeatures device_features;
	vkGetPhysicalDeviceFeatures(physical_device, &device_features);

	VkPhysicalDeviceProperties device_properties;
	vkGetPhysicalDeviceProperties(physical_device, &device_properties);

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

	VkDeviceCreateInfo device_create_info = { 0 };
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.pNext = NULL;
	device_create_info.flags = 0;
	device_create_info.queueCreateInfoCount = 1;
	device_create_info.pQueueCreateInfos = &queue_create_info;
	device_create_info.enabledLayerCount = 0;
	device_create_info.ppEnabledLayerNames = NULL;
	device_create_info.enabledExtensionCount = 0;
	device_create_info.ppEnabledExtensionNames = NULL;
	device_create_info.pEnabledFeatures = NULL;

	VkDevice logical_device;
	result = vkCreateDevice(physical_device, &device_create_info, NULL, &logical_device);
	assert(result == VK_SUCCESS);

	VkQueue queue;
	vkGetDeviceQueue(logical_device, queue_family_index, 0, &queue);

	// Window block
	// -> TODO: Clean up.
	// -> TODO: Remove magic numbers.
	// -> TODO: Architecture?
	// --> Abstraction layers?
	// --> Try dynamic linkage of behavior (movement, AI etc.) for hot loading code.
	// --> Consider consequences of different game object implementations.
	// ---> Consider SoA vs. AoS.
	// -> TODO: Game loop.

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
		L"windowClass",
		L"vulkan",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		500,
		100,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	ShowWindow(window, nShowCmd);
	
	// dynamic behavior linkage test
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