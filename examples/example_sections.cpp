#include "../lib/czmut/src/czmut/czmut.h"

TEST_CASE("A test case with sections", "[example_sections]")
{
	static int dummy = 0;
	dummy++;

	// For each SECTION at the same level, the TEST_CASE is executed from the start.
	// This lets you set a test and then lay out sections without having to repeat the same setup code
	SECTION("section A")
	{
		CHECK(dummy==1);
	}

	// This section is entered with a fresh call to the entire test, so "dummy" will have a value of 2
	SECTION("section B")
	{
		CHECK(dummy==2);
	}

	// This section will cause the test run from the start twice, so both section C.2 an C.3 can have a fresh setup,
	// therefore dummy will have a value of 3 and 4
	SECTION("section C")
	{
		CHECK(dummy==3 || dummy==4);

		// This is taken on the first run of section C
		SECTION("section C.2")
		{
			CHECK(dummy==3);
		}

		// This is taken on the second run of section C
		SECTION("section C.3")
		{
			CHECK(dummy==4);
		}
	}
}


