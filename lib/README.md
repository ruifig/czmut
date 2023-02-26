
# CZMUT (Crazygaze's Micro Unit Tests)

CZMUT is a small unit test framework inspired by Catch (https://github.com/catchorg), targetting embedded systems.

# Features

* Arduino support
* Desktop support (Windows, Linux, etc), so you can develop code on desktop for faster iterations
* Other platforms that have a minimal standard C library.
	* None tested at the moment, but the code as very few dependencies, other than requiring a C++17 compiler
* Small code and ram footprint
	* RAM footprint quickly grows as you add more tests, since it needs to declare global objects (the test cases themselves). This might potentially be reduced further in the future, but complicates the code.
* No use of exceptions. Therefore, some more advanced things are not implemented.
* No heap use

# Documentation

Similar to Catch, tests are created with the `TEST_CASE` macro.

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

### `TEST_CASE(Description, Tags)`

Declares a new test. `Description` is used just for logging purposes. `Tags` is used for filtered when performing a run. See `cz::mut::run`.
Tests automatically self-register, and there is nothing else you need to do to make the test available.

### `SECTION(Description)`

Declares a new test section. `Description` is used sonly for logging purposes.
Sections can be nested.

### `CZMUT_LOG(Fmt, ...)`

Generic logging, similar to `printf`.
Used internally, but also publicly available. `Fmt` is automatically wrapped as `F(Fmt)` for AVR platforms on Arduino, to store it in flash memory.

On Arduino (AVR platform), if one of the parameters is a string in flash memory, you can use the `%S` specifier, although it is not recommended, since that same code might not run on other platforms. This is because the AVR standard library reserves the `%S` specifier for this purpose.

Example:

```cpp
// First string is in RAM. Second string is in PROGMEM
`CZMUT_LOG("This is a log test: %s %S", "Hello", F("World!"));
```

If you want to log a mix if parameters where some are strings stored in flash, you should probably use `cz::mut::logN`.

### `flushlog()`

Flushes the log system. This is mostly useful on the Arduino to flush the serial, to make sure you see a particular log before executing the  rest of the code.

### `logN(...)`

Variadic template function that allows printing several arguments in one go.
Contrary to `CZMUT_LOG`, this does not provide a Format string. It's mostly useful if you need to mix several types of arguments including strings in PROGMEM.
Example:

```cpp
int a = 10; int b=20;
const char* c = "Hello ";
cz::mut::logN(c, F("World!"), a, F(","), b);
```

This will log: `Hello World!10,20`

### `getFilename`

Strips the path part of the specified file.
Mostly useful to log the current file excluding the folder(s) part, to reduce noise.
Example:

```cpp
cz::mut::logN("Current file is: ", getFilename(__FILE__));
```
Or preferably, to save RAM on applicable Arduino boards, you can store __FILE__ in program memory:
```cpp
cz::mut::logN("Current file is: ", getFilename(F(__FILE__)));
```

### equals

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


Some differences from Catch
===========================

Due to its simplicity, there are some serious limitations compared to catch. Some that I can think of:

* Check expressions are not decomposed. They are executed as-is, and if they fail, the full expressions is logged as a string
* A failed check expression causes the entire run to halt straight away. This is intentional, since czmut doesn't use exceptions. 
