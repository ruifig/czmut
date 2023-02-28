
# CZMUT (Crazygaze's Micro Unit Tests)

CZMUT is a small unit test framework inspired by Catch (https://github.com/catchorg), targetting embedded systems.

**Requires C++ 17 support**.

# Features

* Support platforms
	* Arduino
		* Raspberry Pi Pico
		* AVR
		* Fairly easy to add support for others, but no documentation at the moment
	* Desktop support
        * Probably just Windows at the moment. Again, fairly easy to support others, but no documentation.
* Minimal dependency on the C standard library
* Small code and ram footprint
	* RAM footprint quickly grows as you add more tests, since it needs to declare global objects (the test cases themselves). This might potentially be reduced further in the future, but complicates the code.
	* At the time of writting, the sample, built for the Arduino Uno (release build), takes about ~421 bytes of ram and 7308 bytes of flash, although a big chunk of that you only pay once (Arduino globals and code)
* No use of exceptions. Therefore, some more advanced things are not implemented.
* No heap use
* Allows compile time filtered of tests (although in a very simplistic way). Look below for `CZMUT_COMPILE_TIME_TAGS`.

# Documentation

Logging is done to the console on desktop platforms, and to the serial port on microcontrollers (e.g: it uses **Serial** on Arduino)

The API is similar to Catch, tests are created with the `TEST_CASE` macro.


```cpp
int dummy = 1;
TEST_CASE("My first test", "[example]")
{
	// Test if dummy is 1, and stops the test if not
	CHECK(dummy==1);
}
```

In addition to test cases, you also have sections ( `SECTION` macro). This allows sharing setup and teardown code.
Example:

```cpp

TEST_CASE("Test with setup and tear down", "[example]")
{
	// setup code
	int a = 3;
	int b = 10;

	SECTION("Test addition")
	{
		b = 3;
		CHECK((a+b)==6);
	}

	SECTION("Test multiplication")
	{
		CHECK((a*b)==30);
	}

	// .. any tear down code
}
```

For each `SECTION`, the `TEST_CASE` is executed from the start. This means that each section is entered with a fresh setup. For example, even though the first section sets set `b` to `3`, the second section will run with a fresh setup and this see `a = 3` and `b = 10`.
Each run through a test will execute one, and only one, leaf section.

Sections can be nested, in which case the parent section can be entered multiple times, once for each leaf section. Nested sections are most useful when you want to share part of the setup.
Example:

```cpp
TEST_CASE("Test with nested sections", "[example]")
{
	int a = 3;
	int b = 10;

	SECTION("Test 3 numbers")
	{
		int c = 3;

		SECTION("Test Addition")
		{
			CHECK((a+b+c) == 16);
		}

		SECTION("Test subtraction")
		{
			CHECK((a-b-c) == -10);
		}
	}

	SECTION("Test multiplication")
	{
		CHECK((a*b)==30);
	}
}

```

## Full API

Public API is available in the `cz::mut` namespace, while internals are in `cz::mut::detail`. Ideally you should NOT directly use anything from `cz::mut::detail`

### Defining tests

#### `TEST_CASE(Description, Tags)`

Declares a new test. `Description` is used just for logging purposes. `Tags` is used for filtered when performing a run. See `cz::mut::run`.
Tests automatically self-register, and there is nothing else you need to do to make the test available.

#### `TEMPLATED_TEST_CASE(Description, Tags, ...)

Similar to `TEST_CASE`, but the test will be templated for the list of specified types, with the type being tested available as
`TestType`.

For example:
```cpp
TEMPLATE_TEST_CASE("A tempated test case", "[example"], uint8_t, uint16_t)
{
	TestType var;
	CZMUT_LOG("%d\n", sizeof(var));
}
```
This will run two tests. One for `uint8_t` and one for `uint16_t`.

#### `SECTION(Description)`

Declares a new test section. `Description` is used sonly for logging purposes.
Sections can be nested.

#### Reducing Flash and RAM usage

As you add tests to your code, you'll notice that other than the test code itself, there is also an small RAM overhead per test and section.
On microcontrollers with very limited ram, such as a ATmega328P (it has 2k of RAM), you might struggle to fit the code you are testing and czmut itself.

To help with this, tests can be filtered at compile time, effectively removing the Flash and RAM overhead of those tests.
You can achieve this by setting the `CZMUT_COMPILE_TIME_TAGS` macro either for the entire project, or before a chunk of tests.

Setting `CZMUT_COMPILE_TIME_TAGS` to a value of `""` will compile in all tests, while for any other value, a test will be be compiled if its tags contain that test as-is.

Example:

* `#define CZMUT_COMPILE_TIME_TAGS ""` : All tests defined after this will be compiled in
* `#define CZMUT_COMPILE_TIME_TAGS "[mylib]"` : Any tests defined after this will only be compiled in if its tags contain `[mylib]`

**NOTE**: There is no special processing for the value specified in `CZMUT_COMPILE_TIME_TAGS`. If a test's tags does not contain that string, it will be compiled out.

An advantage of using `CZ_MUT_COMPILE_TIME_TAGS` is that you can have a code base with all the unit tests for a given library without worrying about Flash or RAM usage, and selectively compile in only the tests you want.


### Assertion macros

#### `CHECK(expression)`

Executes `expression` and if the result is false it fails the current test case. Execution continues.

#### `REQUIRE(expression)`

Executes `expression` and if the result is false stops execution right away.

Note that contrary to Catch2, a failed REQUIRE does NOT continue to the next test case. It stops the program. This is intentional to avoid C++ exceptions.

#### equals

Compares two provided lists and returns true if they are equal, or false if not.
A couple of  overloads are provided for convenience. (Check the code for available overloads)

Example:
```cpp
TEST_CASE("My test", "[example]")
{
	int a[2] = {1, 2};
	CHECK(cz::mut::equals(a, 2, {1,2}));
}
```

Note that element type of the first list doesn't need to be the same element type of the second list.
As long as said types can be compared with `operator==`, it will work.

More comprehensive overloads might be available in the future, although it would introduce more dependencies, particularly on the C++ STL which is not readily available on some platforms.

### Logging

#### `CZMUT_LOG(Fmt, ...)`

Generic logging, similar to `printf`.
Used internally, but also publicly available. `Fmt` is automatically wrapped as `F(Fmt)` for AVR platforms on Arduino, to store it in flash memory.

On Arduino (AVR platform), if one of the parameters is a string in flash memory, you can use the `%S` specifier, although it is not recommended, since that same code might not run on other platforms. This is because the AVR standard library reserves the `%S` specifier for this purpose.

Example:

```cpp
// First string is in RAM. Second string is in PROGMEM
`CZMUT_LOG("This is a log test: %s %S", "Hello", F("World!"));
```

If you want to log a mix if parameters where some are strings stored in flash, you should probably use `cz::mut::logN`.

#### `flushlog()`

Flushes the log system. This is mostly useful on the Arduino to flush the serial, to make sure you see a particular log before executing the  rest of the code.

#### `logN(...)`

Variadic template function that allows printing several arguments in one go.
Contrary to `CZMUT_LOG`, this does not provide a Format string. It's mostly useful if you need to mix several types of arguments including strings in PROGMEM.
Example:

```cpp
int a = 10; int b=20;
const char* c = "Hello ";
cz::mut::logN(c, F("World!"), a, F(","), b);
```

This will log: `Hello World!10,20`

#### `getFilename`

Strips the path part of the specified file.
Mostly useful to log the current file excluding the folder(s) part, to reduce logging noise.
Example:

```cpp
cz::mut::logN("Current file is: ", getFilename(__FILE__));
```
Or preferably, to save RAM on applicable Arduino boards, you can store __FILE__ in program memory:
```cpp
cz::mut::logN("Current file is: ", getFilename(F(__FILE__)));
```

### Running tests

#### `cz::mut::run( const char* tags )`

Runs all tests that match the specified tag expression.
Returns `true` if all tests passed, `false` if no tests ran or if any tests failed.

czmut's tag expression parsing is rather simplified when compared to Catch2.

* A series of tags form an AND expression, wheras a comma-separated sequence forms an OR expression.
* A `~` before a series of tags negates that series. As-in, it will match all tests that don't satisfy that series.
For example, given the following tests:

```cpp
TEST_CASE("Test 1", "[A][Z]")
{
}

TEST_CASE("Test 2", "[A][B]")
{
}

TEST_CASE("Test 3", "[B][C]")
{
}
```

* A tag expression of `"[B][A]"` will enable only test `Test 2`. Note that the order of the tags is irrelevant. `Test 2` has both the tag `[A]` and `[B]`.
* A tag expression of `"[A]"` will enable tests `Test 1` and `Test 2`.
* A tag expression of `"[A][Z],[C]"` will enable `Test 1` and `Test 3`
    * NOTE: There are two tokens that can be satisfied (`[A][Z]` and `[C]`).
* A tag expression of `"~[A]"` means *Match all tests that don't contain `[A]`*, therefore enabling only `Test 3`.
* A tag expression of `"~[A][Z]"` means *Match all tests that don't contain `[A]` and `[Z]`*, therefore enabling `Test 2` and `Test 3`
* A tag expression of `"~[A][Z],[Z]"` means *Match all tests that don't contain `[A]` and `[Z]`, OR that contain `[Z]`*, therefore enabling all 3 tests.

For now, contrary to Catch2, there is no matching by test description or the special character `*'.

Some differences from Catch
===========================

Due to its simplicity, there are some serious limitations compared to catch. Some that I can think of:

* Assertion expressions are not decomposed. They are executed as-is, and if they fail, the full expressions is logged as a string
* A failed REQUIRE causes the entire run to halt straight away. This is intentional, since czmut doesn't use exceptions. 
* Test selection (with the tags expression), is rather simple. See the documentation for `cz::mut::run`

