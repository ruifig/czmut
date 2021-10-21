void operator delete(void* ptr, unsigned int size)
{
	free(ptr);
}

void setup() {
	Serial.begin(115200);
	while (!Serial) {
		; // wait for serial port to connect. Needed for native USB port only
	}

	CZMUT_LOG("Test %d %s %S %d\n", 10, "Hello", F("World!"), 20);

	CZMUT_LOG("Up and running...");

	int a = 10;
	int b = 20;
	const char* c = "Hello ";

	cz::mut::logN(c, F("World!"), a, F(","), b);
	cz::mut::run(F("[example]"));
}

void loop() {
  // put your main code here, to run repeatedly:
}
