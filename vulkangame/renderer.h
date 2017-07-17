#pragma once
#include "vulkaninfo.h"

void r_initVulkan(OsParams params);
void r_destroyVulkan();
GpuInfo r_test_getGpuInfo();
DeviceInfo r_test_getDeviceInfo();
CommandPoolInfo r_test_getCommandPoolInfo();
PresentInfo r_test_getPresentInfo();
SwapchainInfo r_test_getSwapchainInfo();
void r_renderFrame();
void r_readShaders();
void r_handleOSMessages();
void r_showWindow();