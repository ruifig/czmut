#include "../lib/czmut/src/czmut/czmut.h"

template<typename A, typename B>
struct Pair
{
	A a;
	B b;
};

namespace
{
	void printValue(char value)
	{
		CZMUT_LOG("printing char% d\n", (int)value);
	}

	void printValue(int value)
	{
		CZMUT_LOG("printing int %d\n", value);
	}

}

/*
This test is templated and will be called for types "char" and "int".
*/
TEMPLATED_TEST_CASE("A test case with sections", "[example_sections]", char, int)
{
	// TestType is the type the test case is running for.
	TestType dummy = 10;

	// This will be called for type char and then for type int
	printValue(dummy);
}

