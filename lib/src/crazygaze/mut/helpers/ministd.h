#pragma once

namespace cz::mut::ministd
{
	////////////////////////////////////////////////////////////
	//   What would normally come from <type_traits>
	////////////////////////////////////////////////////////////
	
	// integral_constant
	template<class T, T v>
	struct integral_constant {
		static constexpr T value = v;
		using value_type = T;
		using type = integral_constant; // using injected-class-name
		constexpr operator value_type() const noexcept { return value; }
		constexpr value_type operator()() const noexcept { return value; } //since c++14
	};

	using true_type = integral_constant<bool, true>;
	using false_type = integral_constant<bool, false>;
	
	// remove_reference
	template< class T > struct remove_reference	  {typedef T type;};
	template< class T > struct remove_reference<T&>  {typedef T type;};
	template< class T > struct remove_reference<T&&> {typedef T type;};
	
	// is_lvalue_reference
	template<class T> struct is_lvalue_reference	 : false_type {};
	template<class T> struct is_lvalue_reference<T&> : true_type {};
	
	////////////////////////////////////////////////////////////
	//   What would normally come from <utility>
	////////////////////////////////////////////////////////////
	///
	// Copied from https://stackoverflow.com/questions/27501400/the-implementation-of-stdforward
	template <class T>
	inline T&& forward(typename remove_reference<T>::type& t) noexcept
	{
		 return static_cast<T&&>(t);
	}

	template <class T>
	inline T&& forward(typename remove_reference<T>::type&& t) noexcept
	{
		 static_assert(!is_lvalue_reference<T>::value, "Can not forward an rvalue as an lvalue.");
		 return static_cast<T&&>(t);
	}

}

