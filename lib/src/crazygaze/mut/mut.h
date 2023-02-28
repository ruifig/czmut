#pragma once

#include "static_string.h"

#if __cplusplus < 201703L
	#error "czmut needs C++17 support. On GCC, you can user the build flags -std=c++17 and -std=gnu++17. With MSVC you need /Zc:__cplusplus and /std:c++17"
#endif

#ifdef _WIN32
	#define CZMUT_DESKTOP 1
#else
	#define CZMUT_DESKTOP 0
#endif

#if defined(__AVR__)
	#define CZMUT_AVR 1
#else
	#define CZMUT_AVR 0
#endif

#if defined(ARDUINO_ARCH_RP2040)
	#define CZMUT_RP2040 1
#else
	#define CZMUT_RP2040 0
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

#define CZMUT_USE_OWN_INITIALIZER_LIST 0

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
	#include "./helpers/initializer_list"
#endif

#include "./helpers/ministd.h"

#include <stdio.h>
#include <string.h>
#include "./helpers/vaargs_to_string_array.h"

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

namespace cz::mut
{
	/*
	* Run tests
	* \param tags
	* This allows filtering what tests to run. If empty or null, all test will run. Check the documentation for more details
	*/
	bool run(const __FlashStringHelper* tags = nullptr);
}

namespace cz::mut::detail
{
	void debugbreak();

	void logStr(const char* str);
#if defined(ARDUINO)
	void logStr(const __FlashStringHelper* str);
#endif
	void logFmt(const __FlashStringHelper* fmt, ...);
	void flushlog();

#if defined(ARDUINO)
	void log(const __FlashStringHelper* str);
#endif
	void log(const char* str);
	void log(int val);
	void log(unsigned int val);
	void log(long val);
	void log(unsigned long val);

	void doCheck(bool result, const __FlashStringHelper* file, int line, const __FlashStringHelper* expr_str);
	void doRequire(bool result, const __FlashStringHelper* file, int line, const __FlashStringHelper* expr_str);

	// NOTE: No need for a version of logAssertionFailure that takes "const char* expr_str".
	//	* On Arduino, we'll always use __FlashStringHelper
	//	* On other platforms, __FlashStringHelper is "char", so no need for anything else
	void logAssertionFailure(const __FlashStringHelper* assertionType, const __FlashStringHelper* file, int line, const __FlashStringHelper* expr_str);

	void logFinalResults();

	//
	// Helper to make it easier to manipulate strings in flash memory
	// 
	class FlashStringIterator
	{
		public:
		explicit FlashStringIterator(const __FlashStringHelper* pos)
			: m_pos(reinterpret_cast<const char*>(pos))
		{
		}

		inline FlashStringIterator& operator++()
		{
			m_pos++;
			return *this;
		}

		inline FlashStringIterator& operator--()
		{
			m_pos--;
			return *this;
		}

		inline char operator*() const
		{
		#if defined(ARDUINO)
			return pgm_read_byte(m_pos);
		#else
			return *m_pos;
		#endif
		}

		size_t len() const
		{
		#if defined(ARDUINO)
			return strlen_P(m_pos);
		#else
			return strlen(m_pos);
		#endif
		}

		/**
		 * Find the first occurrence of the specified character.
		 * Returns an iterator to the character, or a null iterator if not found.
		 */
		FlashStringIterator findChar(char ch, size_t pos = 0) const
		{
		#if defined(ARDUINO)
			return FlashStringIterator(strchr_P(m_pos + pos, ch));
		#else
			return FlashStringIterator(strchr(m_pos + pos, ch));
		#endif
		}

		/**
		 * Find the last occurrence of the specified character.
		 * Returns an iterator to the character, or a null iterator if not found.
		 */
		FlashStringIterator findLastChar(char ch) const
		{
		#if defined(ARDUINO)
			return FlashStringIterator(strrchr_P(m_pos, ch));
		#else
			return FlashStringIterator(strrchr(m_pos, ch));
		#endif
		}


		inline bool operator==(const FlashStringIterator& other) const { return m_pos == other.m_pos; }
		inline bool operator!=(const FlashStringIterator& other) const { return m_pos != other.m_pos; }
		inline int operator-(const FlashStringIterator& other) const { return m_pos - other.m_pos; }
		inline FlashStringIterator operator+(int val) const { return FlashStringIterator(m_pos+val); }
		inline FlashStringIterator operator-(int val) const { return FlashStringIterator(m_pos-val); }
		inline FlashStringIterator operator+(size_t val) const { return FlashStringIterator(m_pos+val); }
		inline FlashStringIterator operator-(size_t val) const { return FlashStringIterator(m_pos-val); }
		inline const char* c_str() const { return m_pos; }
		inline const __FlashStringHelper* data() const { return reinterpret_cast<const __FlashStringHelper*>(m_pos); }
		explicit inline operator bool() const { return m_pos ? true : false; }
		bool operator<(const FlashStringIterator& other) { return m_pos < other.m_pos; }
		bool operator>(const FlashStringIterator& other) { return m_pos > other.m_pos; }

	private:
	#if defined(ARDUINO)
		explicit FlashStringIterator(const char* pos_P)
			: m_pos(pos_P)
		{
		}
	#endif
		const char* m_pos;
	};

	//
	// Allows indexing an initializer list, and counting elements
	template<class T>
	struct InitializerListHelper
	{
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

	class Section
	{
	public:
		explicit Section(const __FlashStringHelper* name);
		~Section();

		static Section* getActive();
		const __FlashStringHelper* getName() const;
		bool tryExecute();
		void start();
		void end();

	private:

		enum class State : unsigned char
		{
			Uninitialized,
			Ready,
			Running,
			Finished
		};

		void onChildStart();
		void onChildEnd(State childState);

		const __FlashStringHelper* m_name;
		int m_level = 0;
		bool m_childExecuted = false;
		bool m_hasActiveChild = false;
		State m_state = State::Uninitialized;
		Section* m_parent = nullptr;
		static Section* ms_active;
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
			const __FlashStringHelper* typeName = nullptr;
		};
		
		TestCase(const __FlashStringHelper* name, const __FlashStringHelper* tags);
		virtual ~TestCase() ;
		static TestCase* getActive();
		static const __FlashStringHelper* getActiveTestType();
		const __FlashStringHelper* getName() const;

		bool hasFailed() const
		{
			return m_failed;
		}

		void setFailed()
		{
			m_failed = true;
		}

	protected:
		friend bool cz::mut::run(const __FlashStringHelper* tags);

		virtual void onEnter() {}
		virtual void onExit() {}

		bool hasTag(detail::FlashStringIterator tagStart, detail::FlashStringIterator tagEnd) const;
		static bool run();
		static bool filter(detail::FlashStringIterator tags);

		void setEntries(Entry* entries, unsigned char count)
		{
			m_entries = entries;
			m_numEntries = count;
		}

		template<typename A0>
		void addTypeNames(int index, A0&& a)
		{
			m_entries[index].typeName = a;
		}

		template<typename A0, typename... AN>
		void addTypeNames(int index, A0&& a, AN&&... aN)
		{
			m_entries[index].typeName = a;
			addTypeNames(index+1, ministd::forward<AN>(aN)...);
		}

	private:

		const __FlashStringHelper* m_name;
		const __FlashStringHelper* m_tags;
		TestCase* m_next;
		Entry* m_entries;
		unsigned char m_numEntries;
		bool m_enabled;
		bool m_failed;

		static TestCase* ms_first;
		static TestCase* ms_last;
		static TestCase* ms_active;
		static Entry* ms_activeEntry;
	};

	template<bool enabled> class SingleEntryTestCase;

	template<> class SingleEntryTestCase<false>
	{
	public:
		using EntryFunction = void(*)();
		SingleEntryTestCase(const __FlashStringHelper*, const __FlashStringHelper*, EntryFunction)
		{
		}
	};
	
	template<>
	class SingleEntryTestCase<true> : public TestCase
	{
	public:
		
		SingleEntryTestCase(const __FlashStringHelper* name, const __FlashStringHelper* tags, EntryFunction func)
			: TestCase(name, tags)
			, m_myEntries{ func }
		{
			setEntries(m_myEntries, 1);
		}

	private:
		Entry m_myEntries[1];
	};

	struct Results
	{
		int testsRan = 0;
		int testsSkipped = 0;
		int testsFailed = 0;
		int assertions = 0;
		int assertionsFailed = 0;
	};

	extern Results gResults;
} // cz::mut::detail

namespace cz::mut
{
	/**
	 * Flogs the log.
	 * This is useful on the arduino, whenever you want to make sure some logging is transmitted over the serial before executing the next instruction.
	 */
	void flushlog();

	template<typename A0>
	void logN(A0&& a0)
	{
		detail::log(ministd::forward<A0>(a0));
	}

	/**
	 * Logs any number of passed parameters.
	 * Note that this doesn't support any formating, but makes it easier to log something when you want to mix in strings stored in flash
	 *
	 * int a = 10; int b=20;
	 * const char* c = "Hello ";
	 * cz::mut::logN(c, F("World!"), a, F(","), b);
	 *
	 * This will log: Hello World!10,20
	 *
	 */
	template<typename A0, typename... AN>
	void logN(A0&& a0, AN&&... aN)
	{
		detail::log(ministd::forward<A0>(a0));
		logN(ministd::forward<AN>(aN)...);
	}

	/**
	* Given a file full path, such as given by __FILE__, it will strip the folders and return just the file name
	* Useful to use directly with __FILE__ for logging purposes to reduce noise by not displaying folders.
	*/
	const char* getFilename(const char* file);
	#if defined(ARDUINO)
	const __FlashStringHelper* getFilename(const __FlashStringHelper* file);
	#endif

	// a function, with the short name _ (underscore) for creating 
	// InitializerListHelper out of a "regular" std::initializer_list
	template<class T>
	detail::InitializerListHelper<T> _(const std::initializer_list<T>& list) {
		return detail::InitializerListHelper<T>(list);
	}

	template<typename A, typename B>
	bool equals(A a, B b);

	template<typename A, typename B>
	bool equals(std::initializer_list<A> a, std::initializer_list<B> b)
	{
		size_t a_count = _(a).size();
		size_t b_count = _(b).size();

		if (a_count != b_count)
		{
			return false;
		}

		for (size_t idx = 0; idx < a_count; idx++)
		{
			if (_(a)[idx] != _(b)[idx])
			{
				return false;
			}
		}

		return true;
	}

	template<typename A, typename B>
	bool equals(const A* a, size_t a_count, std::initializer_list<B> b)
	{
		size_t b_count = _(b).size();

		if (a_count != b_count)
		{
			return false;
		}

		for (size_t idx = 0; idx < a_count; idx++)
		{
			if (a[idx] != _(b)[idx])
			{
				return false;
			}
		}

		return true;
	}

	template<typename A, typename B>
	bool equals(const A* a, size_t a_count, const B* b, size_t b_count)
	{
		if (a_count != b_count)
		{
			return false;
		}

		for (size_t idx = 0; idx < a_count; idx++)
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
	if (static cz::mut::detail::Section SectionName(F(Description)); auto CZMUT_ANONYMOUS_VARIABLE(CZMUT_autosection) = cz::mut::detail::AutoSection(SectionName))

#define INTERNAL_TEST_CASE(TestClass, Description, Tags, TestFunction) \
	static void TestFunction(); \
	namespace { \
		static const char CZMUT_CONCATENATE(desc_,TestFunction)[] PROGMEM = Description; \
		static const char CZMUT_CONCATENATE(tags_,TestFunction)[] PROGMEM = Tags; \
		TestClass<cz::mut::contains(cz::mut::StaticString(Tags), cz::mut::StaticString(COMPILE_TIME_TAGS))> CZMUT_ANONYMOUS_VARIABLE(CZMUT_testcase) ( \
			(const __FlashStringHelper*) CZMUT_CONCATENATE(desc_, TestFunction), \
			(const __FlashStringHelper*) CZMUT_CONCATENATE(tags_, TestFunction), \
			&TestFunction); \
	} \
	static void TestFunction() { \
		if constexpr(cz::mut::contains(cz::mut::StaticString(Tags), cz::mut::StaticString(COMPILE_TIME_TAGS)))



	namespace cz::mut::detail
	{
		template<bool enabled, typename TBaseTestClass, int NumEntries>
		struct TemplatedTestCaseBaseClass; 

		template<typename TBaseTestClass, int NumEntries>
		struct TemplatedTestCaseBaseClass<false, TBaseTestClass, NumEntries>
		{
			enum { Enabled = false };
			template<typename... FuncPtrs>
			TemplatedTestCaseBaseClass(const __FlashStringHelper* desc, const __FlashStringHelper* tags, FuncPtrs... funcs) {}
		};

		template<typename TBaseTestClass, int NumEntries>
		struct TemplatedTestCaseBaseClass<true, TBaseTestClass, NumEntries> : public TBaseTestClass
		{
			enum { Enabled = true };
			TestCase::Entry m_myEntries[NumEntries];
			using EntryFunction = void(*)();

			template<typename... FuncPtrs>
			TemplatedTestCaseBaseClass(const __FlashStringHelper* desc, const __FlashStringHelper* tags, FuncPtrs... funcs)
				: TBaseTestClass(desc, tags)
				, m_myEntries { funcs... }
			{
				static_assert(NumEntries == sizeof...(FuncPtrs));
				this->setEntries(m_myEntries, NumEntries);
			}
		};
	}

#if 0
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
				addTypeNames(0, CZMUT_BUILD_STRING_LIST_P(__VA_ARGS__)); \
			} \
		}; \
		TestClass< __VA_ARGS__ > CZMUT_ANONYMOUS_VARIABLE(CZMUT_testcase); \
	} \
	template<typename TestType> \
	static void TestFunction()
#else
#define INTERNAL_TEMPLATED_TEST_CASE(TestClass, BaseTestClass, Description, Tags, TestFunction, ...) \
	template<typename TestType> \
	static void TestFunction(); \
	namespace { \
		template<typename... Type> \
		struct TestClass : public ::cz::mut::detail::TemplatedTestCaseBaseClass<::cz::mut::contains(cz::mut::StaticString(Tags), cz::mut::StaticString(COMPILE_TIME_TAGS)), BaseTestClass, sizeof...(Type)> \
		{ \
			using Super = ::cz::mut::detail::TemplatedTestCaseBaseClass<::cz::mut::contains(cz::mut::StaticString(Tags), cz::mut::StaticString(COMPILE_TIME_TAGS)), BaseTestClass, sizeof...(Type)>;\
			TestClass() \
				: Super(F(Description), F(Tags), (&TestFunction<Type>)...) \
			{ \
				if constexpr(Super::Enabled) \
					Super::addTypeNames(0, CZMUT_BUILD_STRING_LIST_P(__VA_ARGS__)); \
			} \
		}; \
		TestClass< __VA_ARGS__ > CZMUT_ANONYMOUS_VARIABLE(CZMUT_testcase); \
	} \
	template<typename TestType> \
	static void TestFunction()
#endif


#define INTERNAL_CHECK(expr, file, line) \
	cz::mut::detail::doCheck((expr), file, line, F(#expr))

#define INTERNAL_REQUIRE(expr, file, line) \
	cz::mut::detail::doRequire((expr), file, line, F(#expr))

#define TEST_CASE(Description, Tags) INTERNAL_TEST_CASE(cz::mut::detail::SingleEntryTestCase, Description, Tags, CZMUT_ANONYMOUS_VARIABLE(CZMUT_testfunc))

#define TEMPLATED_TEST_CASE(Description, Tags, ...)  \
	INTERNAL_TEMPLATED_TEST_CASE(CZMUT_ANONYMOUS_VARIABLE(CZMUT_TemplateTestCase), cz::mut::detail::TestCase, Description, Tags, CZMUT_ANONYMOUS_VARIABLE(CZMUT_testfunc), __VA_ARGS__)
	
/**
 * Allows specifying a custom class derived from cz::mut::detail::TestCase
 * This allows for example to have custom code on test case entry and exit by overriding TestCase::onEnter/TestCase::onExit
 */
#define CUSTOM_TEST_CASE(TestClass, Description, Tags) \
	INTERNAL_TEST_CASE(TestClass, Description, Tags, CZMUT_ANONYMOUS_VARIABLE(CZMUT_testfunc))

#define CUSTOM_TEMPLATED_TEST_CASE(TestClass, Description, Tags, ...)  \
	INTERNAL_TEMPLATED_TEST_CASE(CZMUT_ANONYMOUS_VARIABLE(CZMUT_TemplatedTestCase), TestClass, Description, Tags, CZMUT_ANONYMOUS_VARIABLE(CZMUT_testfunc), __VA_ARGS__)

#define SECTION(Description) INTERNAL_SECTION(Description, CZMUT_ANONYMOUS_VARIABLE(CZMUT_section))

#define CHECK(expr) INTERNAL_CHECK(expr, cz::mut::getFilename(F(__FILE__)), __LINE__)

#define REQUIRE(expr) INTERNAL_REQUIRE(expr, cz::mut::getFilename(F(__FILE__)), __LINE__)

#define CZMUT_LOG(fmt,...) cz::mut::detail::logFmt(F(fmt), ## __VA_ARGS__)
