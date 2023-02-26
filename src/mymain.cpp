#include <crazygaze/mut/mut.h>

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
