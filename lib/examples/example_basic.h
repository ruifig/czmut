#include <crazygaze/mut/mut.h>

// Required to facilitate compile time test case filtering (see documentation)
#ifndef CZMUT_COMPILE_TIME_TAGS
	#define CZMUT_COMPILE_TIME_TAGS ""
#endif

TEST_CASE("Very simple test", "[example][basic]")
{
	int dummy = 1;
	// If dummy is not 1, this will cause the test to fail and stop here
	CHECK(dummy==1);
}
