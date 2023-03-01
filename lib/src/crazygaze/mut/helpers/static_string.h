#pragma once

#define CZMUT_HAS_EXCEPTION 0

namespace cz::mut
{

#if CZMUT_HAS_EXCEPTION
	static constexpr unsigned requires_inRange(unsigned i, unsigned len)
	{
		return i>= len ? throw "Out of range" : i;
	}

	template< unsigned N >
	static constexpr char nth_char(const char(&arr)[N], unsigned i)
	{
		return requires_inRange(i, N - 1),
			arr[i];
	}
#endif


	class StaticString
	{
		const char* const begin_;
		int size_;
	public:
		template< int N >
		constexpr StaticString(const char(&arr)[N]) : begin_(arr), size_(N - 1) {
			static_assert(N >= 1, "not a string literal");
		}

		constexpr StaticString(const char* const str, int size)
			: begin_(str), size_(size)
		{
		}

#if CZMUT_HAS_EXCEPTION
		constexpr char operator[](int i) const {
			return requires_inRange(i, size_), begin_[i];
		}

		constexpr char at(int i) const {
			return requires_inRange(i, size_), begin_[i];
		}
#endif

		constexpr StaticString subString(int pos) const
		{
			return StaticString(begin_+1, size_-1);
		}

		constexpr auto begin() const -> const char* const {
			return begin_;
		}

		constexpr int size() const {
			return size_;
		}
	};

#if CZMUT_HAS_EXCEPTION
	static constexpr unsigned count( StaticString str, char c, unsigned i = 0, unsigned ans = 0 )
	{
		return i == str.size() ? ans :
				   str[i] == c ? count(str, c, i + 1, ans + 1) :
								 count(str, c, i + 1, ans);
	}
#endif

	static constexpr bool startsWith(StaticString where, StaticString what)
	{
		return where.size()==0 && what.size()!=0
			? false
			: what.size()==0
				? true
				: *where.begin()==*what.begin()
					? startsWith(where.subString(1), what.subString(1))
					: false;
	}

	static constexpr bool contains(StaticString where, StaticString what)
	{
		return where.size()==0 && what.size()!=0
			? false
			: startsWith(where, what)
				? true
				: contains(where.subString(1), what);
	}

}