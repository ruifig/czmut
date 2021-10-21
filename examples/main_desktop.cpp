#include "../lib/czmut/src/czmut/czmut.h"

TEST_CASE("Der Derp ", "[Foo][Bar]")
{
	CZMUT_LOG("Hello world!\n");
	int dummy = 1;

	// If dummy is not 1, this will cause the test to fail and stop here
	CHECK(dummy==1);
}

int main()
{
	return cz::mut::runAll(F("[example],~[basic]")) ? 0 : 1;
}