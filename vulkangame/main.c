#define VK_USE_PLATFORM_WIN32_KHR

#include <Windows.h>
#include <assert.h>
#include <tchar.h>
#include "vulkan\vulkan.h"
#include "datatest.h"
#include "vkrenderer.h"

#define MS_PER_SECOND 1000.0
#define MAX_FRAMERATE 60.0

const double MAX_FRAME_TIME = MS_PER_SECOND / MAX_FRAMERATE;

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
	HWND window = r_initVkRenderer(hInstance, nShowCmd);	
	MSG msg;
	
	while (1)
	{
		if (PeekMessage(&msg, window, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		LARGE_INTEGER startPerformanceCount;
		LARGE_INTEGER endPerformanceCount;
		LARGE_INTEGER performanceFrequency;

		QueryPerformanceCounter(&startPerformanceCount);
		QueryPerformanceFrequency(&performanceFrequency);
		double cpuFrequency = (double)performanceFrequency.QuadPart / 1000.0;	
		
		r_renderFrame();

		QueryPerformanceCounter(&endPerformanceCount);
		double frameTime = (double)(endPerformanceCount.QuadPart - startPerformanceCount.QuadPart)/cpuFrequency;
		
		if (MAX_FRAME_TIME > frameTime)
			Sleep(MAX_FRAME_TIME - frameTime);
			
	}
	
	return msg.wParam;
}
