#pragma once
#include "vulkan\vulkan.h"

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