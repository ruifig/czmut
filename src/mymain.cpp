#include <Arduino.h>
#include <czmut/czmut.h>

/*
class TestC
{
  public:
	TestC(const __FlashStringHelper* name, const __FlashStringHelper* tags)
    : name(name), tags(tags)
  {
  }

  const __FlashStringHelper* name;
  const __FlashStringHelper* tags;
};
class SingleTestC : public TestC
{
public:
	SingleTestC(const __FlashStringHelper* name, const __FlashStringHelper* tags, void* func)
		: TestC(name, tags)
		, func(func)
	{
	}
  
  void* func;
	
private:
};

SingleTestC test(F("Hello"), F("There"), nullptr);

#if 0
#define INTERNAL_TEST_C(TestClass, Description, Tags, TestFunction) \
	static void TestFunction(); \
	namespace { TestClass CZMUT_ANONYMOUS_VARIABLE(CZMUT_testcase) (F(Description), F(Tags), &TestFunction); } \
	static void TestFunction()

#define TEST_C(Description, Tags) INTERNAL_TEST_C(SingleTestC, Description, Tags, CZMUT_ANONYMOUS_VARIABLE(CZMUT_testfunc))
#else
#define INTERNAL_TEST_C(Description, Tags) \
	static void TestFunction(); \
	namespace { SingleTestC CZMUT_ANONYMOUS_VARIABLE(CZMUT_testcase) {F(Description), F(Tags), &TestFunction}; } \
	static void TestFunction()

#define TEST_C(Description, Tags) INTERNAL_TEST_C(Description, Tags)

#endif

TEST_C("Hello", "There")
{
}
*/

TEST_CASE("Hello World", "[vector]")
{
}

TEMPLATED_TEST_CASE("Test1", "[vector]", int, double)
{
  cz::mut::detail::_logN(F("Type "), cz::mut::TestCase::getActiveTestType(), F("\n"));
}

void operator delete(void* ptr, unsigned int size)
{
	free(ptr);
}

void setup() {
	// If using avr-stub, we can't use Serial
	Serial.begin(115200);
	while (!Serial) {
		; // wait for serial port to connect. Needed for native USB port only
	}

  Serial.println("Hello world!");
  cz::mut::TestCase::run();
}

void loop() {
  // put your main code here, to run repeatedly:
}