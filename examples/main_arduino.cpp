void operator delete(void* ptr, unsigned int size)
{
	free(ptr);
}

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
