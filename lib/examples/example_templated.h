#include <crazygaze/mut/mut.h>

/*
This test is templated and will be called for types "char" and "int".
*/
TEMPLATED_TEST_CASE("A templated test case", "[example][templated]", char, int)
{
	// TestType is the type the test case is running for.
	TestType dummy = 10;
	CZMUT_LOG("Size of TestType=%d\n", sizeof(dummy));
}

