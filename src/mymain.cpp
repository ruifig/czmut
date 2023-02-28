#include <crazygaze/mut/mut.h>

// Required to facilitate compile time test case filtering (see documentation)
#ifndef CZMUT_COMPILE_TIME_TAGS
	#define CZMUT_COMPILE_TIME_TAGS ""
#endif

/*
Add all examples so I can test them during development
Note that files with tests should normally be in a cpp file. Tests create globals that get registered with the
framework, so no #include directives are needed.
Here, I'm using #include just to make it easier during development of the library.
*/
#include "../lib/examples/example_basic.h"
#include "../lib/examples/example_sections.h"
#include "../lib/examples/example_templated.h"

#if CZMUT_ARDUINO

#if CZMUT_AVR
// Defining this seems to be necessary for AVR. Seems like it's missing somehow.
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
	//
}


#elif CZMUT_DESKTOP

int main()
{
	return cz::mut::run("[example]") ? EXIT_SUCCESS : EXIT_FAILURE;
}

#else
	#error Unknown platform
#endif
