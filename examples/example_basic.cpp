#include "../lib/czmut/src/czmut/czmut.h"

TEST_CASE("Very simple test", "[example][basic]")
{
	CZMUT_LOG("Hello world!\n");
	int dummy = 1;

	// If dummy is not 1, this will cause the test to fail and stop here
	CHECK(dummy==1);
}

