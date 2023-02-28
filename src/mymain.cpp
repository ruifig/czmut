#include <crazygaze/mut/mut.h>
#include <crazygaze/mut/static_string.h>

//#define COMPILE_TIME_TAGS "[hello]"
#define COMPILE_TIME_TAGS "[example]"

#if 0
#define FOO(SomeFunc) \
	static void SomeFunc() {


FOO(SomeFunc)
{
	return;
}
#endif

#if 0
TEST_CASE("Hello", "[example][hello]")
{
	using namespace cz::mut;
	static_assert(*(StaticString("Hello").begin()+1) == 'e');
	static_assert(startsWith(StaticString("Hello"), StaticString("H")));
	static_assert(startsWith(StaticString("Hello"), StaticString("He")));
	static_assert(startsWith(StaticString("Hello"), StaticString("Hello")));
	static_assert(!startsWith(StaticString("Hello"), StaticString("a")));
	static_assert(!startsWith(StaticString("Hello"), StaticString("Hello_")));

	static_assert(contains(StaticString(""), StaticString("")));
	static_assert(!contains(StaticString(""), StaticString("H")));
	static_assert(contains(StaticString("Hello"), StaticString("")));
	static_assert(contains(StaticString("Hello"), StaticString("H")));
	static_assert(contains(StaticString("Hello"), StaticString("He")));
	static_assert(contains(StaticString("Hello"), StaticString("ll")));
	static_assert(contains(StaticString("Hello"), StaticString("Hello")));
	static_assert(contains(StaticString("Hello"), StaticString("ello")));
	static_assert(!contains(StaticString("Hello"), StaticString("Hello_")));

	memset(CZMUT_var, 0, sizeof(CZMUT_var));
}}
#else
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
TEMPLATED_TEST_CASE("A templated test case", "[example][hello2]", char, int)
{
	// TestType is the type the test case is running for.
	TestType dummy = 10;

	// This will be called for type char and then for type int
	printValue(dummy);
}

TEMPLATED_TEST_CASE("A templated test case", "[example][hello3]", char, int)
{
	// TestType is the type the test case is running for.
	TestType dummy = 11;

	// This will be called for type char and then for type int
	printValue(dummy);
}

/*
Add all examples so I can test them during development
Note that files with tests should normally be in a cpp file. Tests create globals that get registered with the
framework, so no #include directives are needed.
Here, I'm using #include just to make it easier during development of the library.
*/
#endif

//#include "../lib/examples/example_basic.h"
//#include "../lib/examples/example_sections.h"
//#include "../lib/examples/example_templated.h"

#if 0
int gSomeVar = 0;
TEST_CASE("Hello world", "[example][test]")
{

SECTION("SECA")
{
	CHECK(gSomeVar == 1);
	CHECK(gSomeVar == 1);
	CHECK(gSomeVar == 1);
	CHECK(gSomeVar == 1);
	CHECK(gSomeVar == 1);
	CHECK(gSomeVar == 1);
	CHECK(gSomeVar == 1);
	CHECK(gSomeVar == 1);
	CHECK(gSomeVar == 1);
	CHECK(gSomeVar == 1);
	CHECK(gSomeVar == 1);
	CHECK(gSomeVar == 1);
	CHECK(gSomeVar == 1);
	CHECK(gSomeVar == 1);

	REQUIRE(gSomeVar == 10);
}

SECTION("SECB")
{
	CHECK(gSomeVar == 2);
}

}
#endif


#if CZMUT_ARDUINO

#if CZMUT_AVR
void operator delete(void* ptr, unsigned int size)
{
	free(ptr);
}
#endif

void setup() {
	Serial.begin(115200);
	while (!Serial) {
		; // wait for serial port to connect. Needed for native USB port only
	}

	cz::mut::run(F("[example]"));
}

void loop() {
  // put your main code here, to run repeatedly:
}

#elif CZMUT_DESKTOP

int main()
{
	return cz::mut::run(F("[example]")) ? 0 : 1;
}

#else
	#error Unknown platform
#endif
