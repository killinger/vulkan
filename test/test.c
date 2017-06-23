#include <Windows.h>
#include "../vulkangame/datatest.h"

__declspec(dllexport) void testprint(DataTest *test);
void testprint(DataTest *test)
{
	test->testA = 3;
	test->testB = 4;
	test->testC = 5;
}