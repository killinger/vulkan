#include <Windows.h>

__declspec(dllexport) void testprint();
void testprint()
{
	OutputDebugString(L"\ntest\ntest\ntest\n\n");
}