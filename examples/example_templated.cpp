#include "../lib/czmut/src/czmut/czmut.h"

template<typename A, typename B>
struct Pair
{
	A a;
	B b;
	Pair(int a) : a(a), b(0) {}
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

	void printValue(Pair<int, char> value)
	{
		CZMUT_LOG("printing Pair<int,char> %d,%d\n", value.a, (int)value.b);
	}

}

/*
This test is templated and will be called for types "char" and "int".
*/
TEMPLATED_TEST_CASE("A templated test case", "[example][templated]", char, int, Pair<int, char>)
{
	// TestType is the type the test case is running for.
	TestType dummy = 10;

	// This will be called for type char and then for type int
	printValue(dummy);
}

