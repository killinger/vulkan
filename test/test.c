#include <Windows.h>
#include "../vulkangame/datatest.h"

__declspec(dllexport) void testprint(DataTest *test);
void testprint(DataTest *test)
{
	test->testA = 1;
	test->testB = 2;
	test->testC = 3;
}