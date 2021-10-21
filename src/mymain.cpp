
#include <czmut/czmut.h>
#include "../examples/main_arduino.cpp"

TEST_CASE("My test", "[example]")
{
	int a[2] = {1, 2};

	CHECK(!cz::mut::equals(a, 2, {1,2}));
}

// Add all examples so I can test them during development
//#include "../examples/example_basic.cpp"
//#include "../examples/example_sections.cpp"
//#include "../examples/example_templated.cpp"

