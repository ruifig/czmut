#include "./czmut.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

// Used internally only for own library development
#define CZMUT_ASSERT(expr) \
	if (!(expr)) \
	{ \
		cz::mut::detail::logN(F("Assert: "), cz::mut::getFilename(__FILE__), ":", __LINE__, ", ", F(#expr)); \
		cz::mut::detail::debugbreak(); \
	}

//
// Platform abstraction details
//
namespace cz::mut::detail
{
	void debugbreak()
	{
		#ifdef _WIN32
			// Win32 debug instruction
			::__debugbreak();
		#elif CZMUT_AVR8
			__asm__ __volatile__("break");
		#else
			#error Unknown or unsupported platform
		#endif

		// Make sure we don't go anywhere and it stops here, since czmut doesn't can't jump out of tests
		while(true) {};
	}

#if CZMUT_DESKTOP
	void logStr(const char* str)
	{
		printf(str);
	}
#elif defined(ARDUINO)
	void logStr(const char* str)
	{
		Serial.print(str);
	}
	void logStr(const __FlashStringHelper* str)
	{
		Serial.print(str);
	}
#else
		#error Unknown or unsupported platform
#endif

	void logFmt(const __FlashStringHelper* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
#if CZMUT_DESKTOP
		vprintf(fmt, args);
#elif CZMUT_ARDUINO
		constexpr int bufSize = 100;
		char buf[bufSize];
		vsnprintf_P(buf, bufSize, (const char*)fmt, args);
		buf[bufSize-1] = 0;
		Serial.print(buf);
#else
		#error Unknown or unsupported platform
#endif

		va_end(args);
	}

	void flushlog()
	{
		#if CZMUT_DESKTOP
			fflush(stdout);
		#elif CZMUT_ARDUINO
			Serial.flush();
		#else
			#error Unknown or unsupported platform
		#endif
	}

	void log(const char* str)
	{
		logStr(str);
	}

#if defined(ARDUINO)
	void log(const __FlashStringHelper* str)
	{
		logStr(str);
	}
#endif

#if CZMUT_DESKTOP // On Visual studio, we need to use _itoa
	#define CZMUT_itoa _itoa
	#define strncmp_P strncmp
	#define strnstr_P strncmp

#else
	#define CZMUT_itoa itoa
#endif

	void log(int val)
	{
		constexpr int bufSize = 2 + 3 * sizeof(val);
		char buf[bufSize];
		CZMUT_itoa(val, buf, 10);
		logStr(buf);
	}
	
	void log(unsigned int val)
	{
		constexpr int bufSize = 1 + 3 * sizeof(val);
		char buf[bufSize];
		CZMUT_itoa(val, buf, 10);
		logStr(buf);
	}

	void log(long val)
	{
		constexpr int bufSize = 2 + 3 * sizeof(val);
		char buf[bufSize];
		CZMUT_itoa(val, buf, 10);
		logStr(buf);
	}
	
	void log(unsigned long val)
	{
		constexpr int bufSize = 1 + 3 * sizeof(val);
		char buf[bufSize];
		CZMUT_itoa(val, buf, 10);
		logStr(buf);
	}

} // cz::mut::detail

namespace cz::mut
{

const char* getFilename(const char* file)
{
	const char* a = strrchr(file, '\\');
	const char* b = strrchr(file, '/');
	const char* c = a > b ? a : b;
	return c ? c+1 : file;
}

void logFailedTest(const char* file, int line)
{
	detail::logN(F("FAILED: Test ["), TestCase::getActive()->getName());
	const __FlashStringHelper* typeName = TestCase::getActive()->getActiveTestType();
	if (typeName)
	{
		detail::logN(F("<"), typeName, F(">"));
	}
	detail::logN(F("]. Section [" ), Section::getActive()->getName(), F("]. "));
	detail::logN(F("Location ["), file, F(":"), line, F("]:\n"));
}

void logFailure(const char* file, int line, const char* expr_str)
{
	logFailedTest(file, line);
	detail::logN(F("    CHECK: "), expr_str, F("\n"));
	CZMUT_FLUSHLOG();
}

#if defined(ARDUINO)
void logFailure(const char* file, int line, const __FlashStringHelper* expr_str)
{
	logFailedTest(file, line);
	detail::logN(F("    CHECK: "), expr_str, F("\n"));
	CZMUT_FLUSHLOG();
}
#endif

//////////////////////////////////////////////////////////////////////////
// TestCase
//////////////////////////////////////////////////////////////////////////
TestCase* TestCase::ms_first;
TestCase* TestCase::ms_last;
TestCase* TestCase::ms_active;
TestCase::Entry* TestCase::ms_activeEntry;

TestCase::TestCase(const __FlashStringHelper* name, const __FlashStringHelper* tags)
	: m_name(name)
	, m_tags(tags)
{
	if (ms_first == nullptr)
	{
		ms_last = ms_first = this;
	}
	else
	{
		ms_last->m_next = this;
		ms_last = this;
	}
}

TestCase::~TestCase()
{
}

#define strcpy_P strcpy

bool TestCase::filter(const __FlashStringHelper* tags_P)
{
	constexpr int tagsBufSize = 100;
	char tags[tagsBufSize];
	if (strlen(tags_P)+1> tagsBufSize)
	{
		CZMUT_LOG("Filter is too big. Either make it shorter, or increase internal buffer size where this message is.");
		return false;
	}

	strcpy_P(tags, tags_P);

	auto setAll = [](bool enabled)
	{
		TestCase* test = ms_first;
		while (test)
		{
			test->m_enabled = enabled;
			test = test->m_next;
		}
	};

	if (!tags || strlen(tags) == 0)
	{
		setAll(true);
		return true;
	}
	else
	{
		setAll(false);
	}

	while(*tags)
	{
		const char* p = strchr(tags, ',');
		if (p == nullptr)
		{
			p = tags+strlen(tags);
		}

		int len = p-tags;

		bool exclude = false;
		if (tags[0] == '~')
		{
			tags++;
			exclude = true;
			len--;
		}

		TestCase* test = ms_first;
		while (test)
		{
			if (strncmp_P(tags, reinterpret_cast<const char*>(test->m_tags), len) == 0)
			{
				if (!exclude)
				{
					test->m_enabled = true;
				}
			}
			else
			{
				if (exclude)
				{
					test->m_enabled = true;
				}
			}

			test = test->m_next;
		}

		tags = p+1;
	}

	return true;
}

bool TestCase::run()
{
	TestCase* test = ms_first;
	ms_active = nullptr;
	ms_activeEntry = nullptr;

	int testsRan = 0;
	int testsSkipped = 0;
	int totalTestCalls = 0;

	while (test)
	{
		if (test->m_enabled)
		{
			testsRan += test->m_numEntries;
			for(int entryIndex=0; entryIndex<test->m_numEntries; entryIndex++)
			{
				ms_activeEntry = &test->m_entries[entryIndex];
				ms_active = test;

				// Print in two steps, because the second one is also PROGMEM
				detail::logN(F("RUNNING: Test ["), test->m_name);
				if (ms_active->getActiveTestType())
				{
					detail::logN(F("<"), ms_active->getActiveTestType(), F(">"));
				}
				detail::logN(F("]\n"));

				while(ms_activeEntry->rootSection.tryExecute())
				{
					totalTestCalls++;
					AutoSection sec(ms_activeEntry->rootSection);
					test->onEnter();
					ms_activeEntry->func();
					test->onExit();
				}
			}
		}
		else
		{
			testsSkipped += test->m_numEntries;
		}

		test = test->m_next;
	}

	detail::logFmt(F("FINISHED : %d tests ran. %d skipped\n"), testsRan, testsSkipped);

	ms_active = nullptr;
	ms_activeEntry = nullptr;
	return true;
}

cz::mut::TestCase* TestCase::getActive()
{
	return ms_active;
}

const __FlashStringHelper* TestCase::getActiveTestType()
{
	return (ms_activeEntry) ? ms_activeEntry->typeName : nullptr;
}

const __FlashStringHelper* TestCase::getName() const
{
	return m_name;
}

//////////////////////////////////////////////////////////////////////////
// Section
//////////////////////////////////////////////////////////////////////////
Section* Section::ms_active;

Section::Section(const __FlashStringHelper* name)
	: m_name(name)
	, m_state(SectionState::Ready)
{
}

Section::~Section()
{
	ms_active = m_parent;
}

cz::mut::Section* Section::getActive()
{
	return ms_active;
}

const __FlashStringHelper* Section::getName() const
{
	return m_name;
}

bool Section::tryExecute()
{
	if ((m_parent && m_parent->m_childExecuted) || (m_state == SectionState::Finished))
	{
		return false;
	}

	m_state = SectionState::Running;
	m_childExecuted = false;
	m_hasActiveChild = false;
	if (m_parent)
	{
		m_parent->m_childExecuted = true;
	}
	return true;
}

void Section::start()
{
	if (ms_active == nullptr)
	{
		m_level = 0;
		ms_active = this;
		m_parent = nullptr;
	}
	else
	{
		m_parent = ms_active;
		ms_active = this;
		m_level = m_parent->m_level + 4;
		m_parent->onChildStart();
	}

	//printf("%*sSection::start() - %s\n", m_level, "", m_name);
}

void Section::end()
{
	if (m_state == SectionState::Ready)
	{
		// If the state is "Ready", it means we are skipping this section. As-in, tryExecute failed
	}
	else if (m_state == SectionState::Running)
	{
		if (m_hasActiveChild)
		{
			m_state = SectionState::Ready;
		}
		else
		{
			m_state = SectionState::Finished;
		}
	}
	else if (m_state == SectionState::Finished)
	{
		//
	}
	else
	{
		CZMUT_ASSERT(false);
	}

	if (m_parent)
	{
		m_parent->onChildEnd(m_state);
		ms_active = m_parent;
	}
	else
	{
		ms_active = nullptr;
	}

	//printf("%*sSection::end() - %s\n", m_level, "", m_name);
}

void Section::onChildStart()
{
	//printf("%*sSection::onChildStart() - %s\n", m_level + 4, "", m_name);
}

void Section::onChildEnd(SectionState childState)
{
	if (childState == SectionState::Ready)
	{
		m_hasActiveChild = true;
	}
	//printf("%*sSection::onChildEnd() - %s\n", m_level + 4, "", m_name);
}

bool runAll(const __FlashStringHelper* tags)
{
	if (!TestCase::filter(tags))
	{
		return false;
	}

	return TestCase::run();
}

} // cz::mut

