#pragma once

#ifdef _WIN32
	#define CZMUT_DESKTOP 1
#else
	#define CZMUT_DESKTOP 0
#endif

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega644__) || defined(__AVR_ATmega644A__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644PA__)
	#define CZMUT_AVR8 1
#else
	#define CZMUT_AVR8 0
#endif

#if defined(ARDUINO)
	#define CZMUT_ARDUINO 1
	#include <Arduino.h>
#else
	#define CZMUT_ARDUINO 0
	using __FlashStringHelper = char;
	#define F(string_literal) string_literal
	#define PROGMEM
#endif

#define CZMUT_USE_OWN_INITIALIZER_LIST 1

//
// From what CZMUT uses of the STL, initializer_list is the tricky one, because it needs to be
// in the std namespace.
// So, if we were to add our own initializer_list that exists in the std namespace, it would conflict
// with the system's own std::initializer_list if the user included it
//
// Therefore if the system <initializer_list> we need to use it
// 
#if __has_include(<initializer_list>) && (!defined(CZMUT_USE_OWN_INITIALIZER_LIST) || CZMUT_USE_OWN_INITIALIZER_LIST==0)
	#include <initializer_list>
#else
	#include "./ministd/initializer_list"
#endif

#include "./ministd/czmut_ministd.h"

#include <stdio.h>

namespace cz::mut::detail
{
	void _debugbreak();

	void _logStr(const char* str);
#if defined(ARDUINO)
	void _logStr(const __FlashStringHelper* str);
#endif

	void _log(const __FlashStringHelper* fmt, ...);
	void _flushlog();

} // cz::mut::detail

#define CZMUT_CONCATENATE_IMPL(s1,s2) s1##s2
#define CZMUT_CONCATENATE(s1,s2) CZMUT_CONCATENATE_IMPL(s1,s2)

// Note: __COUNTER__ Expands to an integer starting with 0 and incrementing by 1 every time it is used in a source file or included headers of the source file.
#ifdef __COUNTER__
	#define CZMUT_ANONYMOUS_VARIABLE(str) \
		CZMUT_CONCATENATE(str,__COUNTER__)
#else
	#define ANONYMOUS_VARIABLE(str) \
		CZMUT_CONCATENATE(str,__LINE__)
#endif

#define CZMUT_FLUSHLOG() cz::mut::detail::_flushlog()

namespace cz::mut
{

const char* getFilename(const char* file);
void logFailure(const char* file, int line, const char* expr_str);

enum class SectionState
{
	Uninitialized,
	Ready,
	Running,
	Finished
};

class Section
{
public:
	Section(const __FlashStringHelper* name);
	~Section();

	static Section* getActive();
	const __FlashStringHelper* getName() const;
	bool tryExecute();
	void start();
	void end();

private:

	void onChildStart();
	void onChildEnd(SectionState childState);

	const __FlashStringHelper* m_name;
	int m_level = 0;
	bool m_childExecuted = false;
	bool m_hasActiveChild = false;
	SectionState m_state = SectionState::Uninitialized;
	Section* m_parent = nullptr;
	static Section* ms_active;
};

class TestCase
{
public:
	using EntryFunction = void(*)();

	struct Entry
	{
		Entry()
			: func {nullptr}
			, rootSection(F("ROOT"))
		{
		}
		
		Entry(EntryFunction func)
			: func(func)
			, rootSection(F("ROOT"))
		{
		}
		EntryFunction func;
		Section rootSection;
	};
	
	TestCase(const __FlashStringHelper* name, const __FlashStringHelper* tags);
	virtual ~TestCase() ;
	static TestCase* getActive();
	static bool run();
	const __FlashStringHelper* getName() const;

	virtual void onEnter() {}
	virtual void onExit() {}

protected:
	void setEntries(Entry* entries, int count)
	{
		m_entries = entries;
		m_numEntries = count;
	}
private:

	const __FlashStringHelper* m_name;
	const __FlashStringHelper* m_tags;
	TestCase* m_next;
	Entry* m_entries;
	int m_numEntries;

	static TestCase* ms_first;
	static TestCase* ms_last;
	static TestCase* ms_active;
};

class SingleEntryTestCase : public TestCase
{
public:
	
	SingleEntryTestCase(const __FlashStringHelper* name, const __FlashStringHelper* tags, EntryFunction func)
		: TestCase(name, tags)
		, m_myEntries{ func }
	{
		setEntries(m_myEntries, 1);
	}
	
	virtual ~SingleEntryTestCase() {}
private:
	Entry m_myEntries[1];
};

class AutoSection
{
public:
	AutoSection(Section& section)
		: m_section(section)
	{
		m_section.start();
	}

	~AutoSection()
	{
		m_section.end();
	}

	explicit operator bool()
	{
		return m_section.tryExecute();
	}

private:
	Section& m_section;
};

//
// Allows indexing an initializer list, and counting elements
template<class T>
struct InitializerListHelper {
	const std::initializer_list<T>& list;
	InitializerListHelper(const std::initializer_list<T>& list) : list(list)
	{
	}

	T operator[](size_t index)
	{
		return *(list.begin() + index);
	}

	size_t size() const
	{
		return list.end() - list.begin();
	}
};

// a function, with the short name _ (underscore) for creating 
// the _init_list_with_square_brackets out of a "regular" std::initializer_list
template<class T>
InitializerListHelper<T> _(const std::initializer_list<T>& list) {
	return InitializerListHelper<T>(list);
}

template<typename A, typename B>
bool compare(A a, B b);

template<typename A, typename B>
bool compare(std::initializer_list<A> a, std::initializer_list<B> b)
{
	size_t a_count = _(a).size();
	size_t b_count = _(b).size();
	
	if (a_count != b_count)
	{
		return false;
	}

	for (int idx = 0; idx < a_count; idx++)
	{
		if (_(a)[idx] != _(b)[idx])
		{
			return false;
		}
	}

	return true;
}

template<typename A, typename B>
bool compare(const A* a, size_t a_count, std::initializer_list<B> b)
{
	size_t b_count = _(b).size();
	
	if (a_count != b_count)
	{
		return false;
	}

	for (int idx = 0; idx < a_count; idx++)
	{
		if (a[idx] != _(b)[idx])
		{
			return false;
		}
	}

	return true;
}

template<typename A, typename B>
bool compare(const A* a, size_t a_count, const B* b, size_t b_count)
{
	if (a_count != b_count)
	{
		return false;
	}

	for (int idx = 0; idx < a_count; idx++)
	{
		if (a[idx] != b[idx])
		{
			return false;
		}
	}

	return true;
}

} // cz::mut

#define INTERNAL_SECTION(Description, SectionName) \
	if (static cz::mut::Section SectionName(Description); auto CZMUT_ANONYMOUS_VARIABLE(CZMUT_autosection) = cz::mut::AutoSection(SectionName))

#define INTERNAL_TEST_CASE(TestClass, Description, Tags, TestFunction) \
	static void TestFunction(); \
	namespace { \
		static const char CZMUT_CONCATENATE(desc_,TestFunction)[] PROGMEM = Description; \
		static const char CZMUT_CONCATENATE(tags_,TestFunction)[] PROGMEM = Tags; \
		TestClass CZMUT_ANONYMOUS_VARIABLE(CZMUT_testcase) ( \
			(const __FlashStringHelper*) CZMUT_CONCATENATE(desc_, TestFunction), \
			(const __FlashStringHelper*) CZMUT_CONCATENATE(tags_, TestFunction), \
			&TestFunction); \
	} \
	static void TestFunction()

#define INTERNAL_TEMPLATED_TEST_CASE(TestClass, BaseTestClass, Description, Tags, TestFunction, ...) \
	template<typename TestType> \
	static void TestFunction(); \
	namespace { \
		template<typename... Type> \
		struct TestClass : public BaseTestClass \
		{ \
			Entry m_myEntries[sizeof...(Type)]; \
			TestClass() \
				: BaseTestClass(F(Description), F(Tags)) \
				, m_myEntries { (&TestFunction<Type>)... } \
			{ \
				setEntries(m_myEntries, sizeof...(Type)); \
			} \
		}; \
		TestClass< __VA_ARGS__ > CZMUT_ANONYMOUS_VARIABLE(CZMUT_testcase); \
	} \
	template<typename TestType> \
	static void TestFunction()

#define INTERNAL_CHECK(expr, file, line) \
	if (!(expr)) \
	{ \
		cz::mut::logFailure(file, line, #expr); \
		cz::mut::detail::_debugbreak(); \
	}

/*
Usage:

TEST_CASE("Foo tests", "[foo]")
{
	... code ...

	SECTION("Section A1")
	{
		... code ...
		SECTION("Section B1")
		{
			... code ...
		}
		... code ...
		SECTION("Section B2")
		{
			... code ...
		}
	}

	SECTION("Section A2")
	{
		... code ...
	}
	... code ...
}
*/

#define TEST_CASE(Description, Tags) INTERNAL_TEST_CASE(cz::mut::SingleEntryTestCase, Description, Tags, CZMUT_ANONYMOUS_VARIABLE(CZMUT_testfunc))

#define TEMPLATED_TEST_CASE(Description, Tags, ...)  \
	INTERNAL_TEMPLATED_TEST_CASE(CZMUT_ANONYMOUS_VARIABLE(CZMUT_TemplateTestCase), cz::mut::TestCase, Description, Tags, CZMUT_ANONYMOUS_VARIABLE(CZMUT_testfunc), __VA_ARGS__)
	
/**
 * Allows specifying a custom class derived from cz::mut::TestCase
 * This allows for example to have custom code on test case entry and exit by overriding TestCase::onEnter/TestCase::onExit
 */
#define CUSTOM_TEST_CASE(TestClass, Description, Tags) \
	INTERNAL_TEST_CASE(TestClass, Description, Tags, CZMUT_ANONYMOUS_VARIABLE(CZMUT_testfunc))

#define CUSTOM_TEMPLATED_TEST_CASE(TestClass, Description, Tags, ...)  \
	INTERNAL_TEMPLATED_TEST_CASE(CZMUT_ANONYMOUS_VARIABLE(CZMUT_TemplatedTestCase), TestClass, Description, Tags, CZMUT_ANONYMOUS_VARIABLE(CZMUT_testfunc), __VA_ARGS__)

#define SECTION(Description) INTERNAL_SECTION(Description, CZMUT_ANONYMOUS_VARIABLE(CZMUT_section))

#define CHECK(expr) INTERNAL_CHECK(expr, cz::mut::getFilename(__FILE__), __LINE__)

#define CZMUT_LOG(fmt, ...) cz::mut::detail::_log(F(fmt), __VA_ARGS__)
#define CZMUT_FLUSHLOG() cz::mut::detail::_flushlog()
