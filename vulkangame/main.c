#include <Windows.h>
#include <assert.h>
#include "vulkan\vulkan.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	VkResult result;

	uint32_t instance_extensions_count;
	result = vkEnumerateInstanceExtensionProperties(NULL, &instance_extensions_count, NULL);
	assert(result == VK_SUCCESS);

	VkExtensionProperties *available_instance_extensions =
		(VkExtensionProperties*)malloc(instance_extensions_count * sizeof(VkExtensionProperties)); // free
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
		(VkPhysicalDevice*)malloc(devices_count * sizeof(VkPhysicalDevice)); // free
	result = vkEnumeratePhysicalDevices(instance, &devices_count, available_devices);
	assert(result == VK_SUCCESS);

	VkPhysicalDevice physical_device = available_devices[0];

	uint32_t device_extensions_count;
	result = vkEnumerateDeviceExtensionProperties(physical_device, NULL, &device_extensions_count, NULL);
	assert(result == VK_SUCCESS);

	VkExtensionProperties *available_device_extensions =
		(VkExtensionProperties*)malloc(device_extensions_count * sizeof(VkExtensionProperties)); // free
	result = vkEnumerateDeviceExtensionProperties(physical_device, NULL, 
		&device_extensions_count, available_device_extensions);
	assert(result == VK_SUCCESS);	

	VkPhysicalDeviceFeatures device_features;
	vkGetPhysicalDeviceFeatures(physical_device, &device_features);

	VkPhysicalDeviceProperties device_properties;
	vkGetPhysicalDeviceProperties(physical_device, &device_properties);

	uint32_t queue_families_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_families_count, NULL);

	VkQueueFamilyProperties *queue_families = 
		(VkQueueFamilyProperties*)malloc(queue_families_count * sizeof(VkQueueFamilyProperties)); // free
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_families_count, queue_families);
	
	return 0;
}