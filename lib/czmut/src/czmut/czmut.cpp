#include "./czmut.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define CZMUT_ASSERT(expr) \
	if (!(expr)) \
	{ \
		cz::mut::detail::_log(F("Assert: %s:%d, %s"), cz::mut::getFilename(__FILE__), __LINE__, #expr); \
		cz::mut::detail::_debugbreak(); \
	}

//
// Platform abstraction details
//
namespace cz::mut::detail
{
	void _debugbreak()
	{
		#ifdef _WIN32
			::__debugbreak();
		#elif CZMUT_AVR8
			__asm__ __volatile__("break");
		#else
			#error Unknown or unsupported platform
		#endif
	}

	void _logStr(const char* str)
	{
#if CZMUT_DESKTOP
		printf(str);
#elif CZMUT_ARDUINO
				constexpr int bufSize = 100;
				char buf[bufSize];
				vsnprintf_P(buf, bufSize, (const char*)fmt, args);
				buf[bufSize-1] = 0;
				Serial.print(buf);
		#else
				#error Unknown or unsupported platform
		#endif
	}

	void _log(const __FlashStringHelper* fmt, ...)
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

	void _flushlog()
	{
		#if CZMUT_DESKTOP
			fflush(stdout);
		#elif CZMUT_ARDUINO
			Serial.flush();
		#else
			#error Unknown or unsupported platform
		#endif
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

void logFailure(const char* file, int line, const char* expr_str)
{
	CZMUT_LOG("Test [%s] failed. Section [%s]. Location [%s:%u]:\n", cz::mut::TestCase::getActive()->getName(), cz::mut::Section::getActive()->getName(), file, line);
	CZMUT_LOG("Expression: %s\n", expr_str);
	CZMUT_FLUSHLOG();
}

//////////////////////////////////////////////////////////////////////////
// TestCase
//////////////////////////////////////////////////////////////////////////
TestCase* TestCase::ms_first;
TestCase* TestCase::ms_last;
TestCase* TestCase::ms_active;

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

bool TestCase::run()
{
	TestCase* test = ms_first;

	int testCount = 0;
	int testPasses = 0;

	while (test)
	{
		testCount++;
		// Print in two steps, because the second one is also PROGMEM
		detail::_log(F("**** Running Test "));
		detail::_log(test->m_name);
		if (test->m_numEntries > 1)
		{
			detail::_log(F(" (%d types)"), test->m_numEntries);
		}
		detail::_log(F("\n"));
		for(int entryIndex=0; entryIndex<test->m_numEntries; entryIndex++)
		{
			Entry& entry = test->m_entries[entryIndex];
			ms_active = test;
			while(entry.rootSection.tryExecute())
			{
				testPasses++;
				AutoSection sec(entry.rootSection);
				test->onEnter();
				entry.func();
				test->onExit();
			}
		}

		test = test->m_next;
	}

	CZMUT_LOG("%d tests ran (%d passes)\n", testCount, testPasses);

	return true;
}

cz::mut::TestCase* TestCase::getActive()
{
	return ms_active;
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
		CZMUT_ASSERT(0);
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

} // cz::mut


#if 0

TEST_CASE("vector tests", "[vector]")
{
	printf("Running %s\n", cz::mut::TestCase::getActive()->getName());

	SECTION("A1")
	{
		printf("	Running A1\n");

		SECTION("B1")
		{
			printf("		Running B1\n");
			SECTION("C1")
			{
				printf("		Running C1\n");
				CHECK(false);
			}
		}

		SECTION("B2")
		{
			printf("		Running B2\n");
		}

		SECTION("B3")
		{
			printf("		Running B3\n");
		}
	}

	SECTION("A2")
	{
		printf("	Running A2\n");
	}
}

TEST_CASE("vector tests2", "[vector]2")
{
	printf("Running %s\n", cz::mut::TestCase::getActive()->getName());
}

#endif