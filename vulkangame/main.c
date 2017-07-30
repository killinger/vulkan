#define VK_USE_PLATFORM_WIN32_KHR

#include <Windows.h>
#include <assert.h>
#include <tchar.h>
//#include "vulkaninfo.h"
#include "vulkan\vulkan.h"
#include "datatest.h"
//#include "renderer.h"
#include "vkrenderer.h"

#define FRAME_TIME 1000.0/60.0

void dllTest()
{
	DataTest test = { 0 };
	HMODULE test_dll = LoadLibrary(L"test.dll");
	void(*func)(DataTest*) = (void*)GetProcAddress(test_dll, "testprint");
	func(&test);
	FreeLibrary(test_dll);
	int i = 0;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	//OsParams params = { 0 };
	//params.hInstance = hInstance;
	//params.hPrevInstance = hPrevInstance;
	//params.lpCmdLine = lpCmdLine;
	//params.nShowCmd = nShowCmd;

	//r_initVulkan(params);
	//r_renderFrame();

	//PresentInfo prsntinfo = r_test_getPresentInfo();
	//SwapchainInfo swapchaininfo = r_test_getSwapchainInfo();

	//r_showWindow();
	//r_handleOSMessages();

	HWND window = r_initVkRenderer(hInstance, nShowCmd);

	MSG msg;
	
	while (1)
	{
		while (PeekMessage(&msg, window, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		DWORD start = GetTickCount();


		Sleep(start + FRAME_TIME - GetTickCount());
	}
	
	return msg.wParam;
}
