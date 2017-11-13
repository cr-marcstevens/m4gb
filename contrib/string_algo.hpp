/*********************************************************************************\
*                                                                                 *
* https://github.com/cr-marcstevens/snippets/tree/master/cxxheaderonly            *
*                                                                                 *
* string_algo.hpp - A header only C++ string algorithms                           *
* Copyright (c) 2017 Marc Stevens                                                 *
*                                                                                 *
* MIT License                                                                     *
*                                                                                 *
* Permission is hereby granted, free of charge, to any person obtaining a copy    *
* of this software and associated documentation files (the "Software"), to deal   *
* in the Software without restriction, including without limitation the rights    *
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell       *
* copies of the Software, and to permit persons to whom the Software is           *
* furnished to do so, subject to the following conditions:                        *
*                                                                                 *
* The above copyright notice and this permission notice shall be included in all  *
* copies or substantial portions of the Software.                                 *
*                                                                                 *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR      *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,        *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE     *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER          *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,   *
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE   *
* SOFTWARE.                                                                       *
*                                                                                 *
\*********************************************************************************/

#ifndef STRING_ALGO_HPP
#define STRING_ALGO_HPP

#include <locale>
#include <string>
#include <vector>
#include <queue>
#include <iterator>
#include <algorithm>
#include <functional>

/****************************** example usage ************************************\
grep "^//test.cpp" string_algo.hpp -A55 > test.cpp
g++ -std=c++11 -o test test.cpp -g -ggdb

//test.cpp:
#include "string_algo.hpp"
#include <iostream>
#include <array>
namespace sa = string_algo;

int main(int argc, char** argv)
{
	std::array<std::string,3> tests = { " a b c d e ", " aA09zZ.!-+", " a + x * f - h = 0 " };
	std::string blah = "blah";
	for (auto& test : tests)
	{
		std::cout << std::endl << "================== Test string: '" << test << "'" << std::endl;
		std::cout << "Count ' ': " << sa::count_pred(test, sa::is_equal(' ')) << " = " << sa::count(test, ' ') << std::endl;
		std::cout << "Counts: lower=" << sa::count_pred(test, sa::is_lower()) << " upper=" << sa::count_pred(test, sa::is_upper()) << " digit=" << sa::count_pred(test,sa::is_digit()) << " punct=" << sa::count_pred(test,sa::is_punct()) << " print=" << sa::count_pred(test,sa::is_print()) << std::endl;
		std::cout << "Modified: " << sa::modify_copy(test, [](char& c){ ++c; }) << std::endl;
		sa::modify(test, [](char& c){c+=0;}); // no op
		std::cout << "Pred: alllower=" << sa::all_lower(test) << " noupper=" << !sa::any_upper(test) << "=" << sa::all_not_pred(test,sa::is_upper()) << std::endl;
		std::cout << "Split+join: '" << sa::join(sa::split(test, ' '),'#') << "', '" << sa::join(sa::split(test, std::string(" +"))) << "'" << std::endl;
		std::cout << "Erased: a='" << sa::erase_pred_copy(test, sa::is_lessequal('a')) << "'" << std::endl;
		std::cout << "tolower='" << sa::to_lower_copy(test) << "' toupper='" << sa::to_upper_copy(test) << "'" << std::endl;
		std::cout << "left='" << sa::left_copy(test,2) << "' mid='" << sa::mid_copy(test,2,2) << "' right='" << sa::right_copy(test,2) << "'" << std::endl;
		auto i1 = sa::find(test, 'a'), i2 = sa::find(test, 'a',test.begin());
		auto i3 = sa::find(test, "a"), i4 = sa::find(test, "a",test.begin());
		if (i1 != i2 || i1 != i3 || i1 != i4) throw;
		auto i5 = sa::ifind(test, 'A'), i6 = sa::ifind(test, 'A',test.begin());
		auto i7 = sa::ifind(test, "A"), i8 = sa::ifind(test, "A",test.begin());
		if (i5 != i6 || i5 != i7 || i5 != i8) throw;
		std::cout << ((i5 == i1) ? "First a/A is a" : "First a/A is A") << std::endl;
		auto il1 = sa::find_last(test, 'a'), il2 = sa::find_last(test, 'a',test.end());
		auto il3 = sa::find_last(test, "a"), il4 = sa::find_last(test, "a",test.end());
		if (il1 != il2 || il1 != il3 || il1 != il4) throw;
		auto il5 = sa::ifind_last(test, 'A'), il6 = sa::ifind_last(test, 'A',test.end());
		auto il7 = sa::ifind_last(test, "A"), il8 = sa::ifind_last(test, "A",test.end());
		if (il5 != il6 || il5 != il7 || il5 != il8) throw;
		std::cout << ((il5 == il1) ? "Last a/A is a" : "Last a/A is A") << std::endl;
		std::cout << sa::starts_with(test, " a ") << " " << sa::istarts_with(test, " A ") << std::endl;
		std::cout << sa::ends_with(test, " e ") << " " << sa::iends_with(test, " E ") << std::endl;
		std::cout << sa::contains(test, "aa") << " " << sa::icontains(test, "aa") << " " << sa::icontains(test, "AA") << std::endl;
		std::cout << sa::equals(test, sa::to_upper_copy(test)) << " " << sa::iequals(test, sa::to_upper_copy(test)) << std::endl;
		std::cout << "Trim: left='" << sa::trim_left_copy(test) << "' right='" << sa::trim_right_copy(test) << "' both='" << sa::trim_copy(test) << "'" << std::endl;
		std::cout << "Trim: left='" << sa::trim_left_copy(test, ' ') << "' right='" << sa::trim_right_copy(test, ' ') << "' both='" << sa::trim_copy(test, ' ') << "'" << std::endl;
		std::cout << "Trim: left='" << sa::trim_left_copy(test, " ") << "' right='" << sa::trim_right_copy(test, " ") << "' both='" << sa::trim_copy(test, " ") << "'" << std::endl;
		std::cout << "Trim ' ae': left='" << sa::trim_left_copy(test, " ae") << "' right='" << sa::trim_right_copy(test, " ae") << "' both='" << sa::trim_copy(test, " ae") << "'" << std::endl;
		std::cout << "Trim: left='" << sa::trim_left_copy(test, ' ') << "' right='" << sa::trim_right_copy(test, ' ') << "' both='" << sa::trim_copy(test, ' ') << "'" << std::endl;
		std::cout << "Replacehead: '" << sa::replace_head_copy(test, 2, blah) << "'" << std::endl;
		std::cout << "Replacetail: '" << sa::replace_tail_copy(test, 2, blah) << "'" << std::endl;
		std::cout << "Insert: '" << sa::insert_copy(test, 1, blah) << "'" << std::endl;
		std::cout << "Erase: '" << sa::erase_copy(test, 1, 2) << "'" << std::endl;
		std::cout << "Erasepred ' ': '" << sa::erase_pred_copy(test, sa::is_equal(' ')) << "'" << std::endl;
		std::cout << "Replacefirst: '" << sa::replace_first_copy(test, std::string("a"), blah) << "'" << std::endl;
		std::cout << "Replacelast: '" << sa::replace_last_copy(test, std::string("a"), blah) << "'" << std::endl;
		std::cout << "Ireplacefirst: '" << sa::ireplace_first_copy(test, std::string("A"), blah) << "'" << std::endl;
		std::cout << "Replaceall: '" << sa::replace_all_copy(test, std::string(" "), std::string()) << "'" << std::endl;
	}
}

\**************************** end example usage **********************************/

namespace string_algo {

	namespace sa = ::string_algo;

	/* ADVANCE COPY */

	template<typename Iterator, typename Distance>
	Iterator advance_copy(const Iterator& it, Distance n)
	{
		Iterator ret(it);
		std::advance(ret, n);
		return ret;
	}

	/* COUNT */

	template<typename String>
	std::size_t count(const String& str, const typename String::value_type& c)
	{
		std::size_t ret = 0;
		auto it = str.begin(), end = str.end();
		for (; it != end; ++it)
			if (*it == c)
				++ret;
		return ret;
	}

	template<typename String>
	std::size_t count_not(const String& str, const typename String::value_type& c)
	{
		std::size_t ret = 0;
		auto it = str.begin(), end = str.end();
		for (; it != end; ++it)
			if (!*it != c)
				++ret;
		return ret;
	}

	template<typename String, typename Pred>
	std::size_t count_pred(const String& str, Pred pred)
	{
		std::size_t ret = 0;
		auto it = str.begin(), end = str.end();
		for (; it != end; ++it)
			if (pred(*it))
				++ret;
		return ret;
	}

	template<typename String, typename Pred>
	std::size_t count_not_pred(const String& str, Pred pred)
	{
		std::size_t ret = 0;
		auto it = str.begin(), end = str.end();
		for (; it != end; ++it)
			if (!pred(*it))
				++ret;
		return ret;
	}

	/* MODIFY */

	template<typename String, typename Mod>
	void modify(String& str, Mod mod)
	{
		for (auto& c : str)
			mod(c);
	}

	template<typename String, typename Mod>
	String modify_copy(const String& str, Mod mod)
	{
		String ret(str);
		for (auto& c : ret)
			mod(c);
		return ret;
	}

	/* ALL PRED */

	template<typename String, typename Pred>
	bool all_pred(const String& str, Pred pred)
	{
		for (auto& c : str)
			if (!pred(c))
				return false;
		return true;
	}

	template<typename String, typename Pred>
	bool all_not_pred(const String& str, Pred pred)
	{
		for (auto& c : str)
			if (pred(c))
				return false;
		return true;
	}

	/* ANY PRED */

	template<typename String, typename Pred>
	bool any_pred(const String& str, Pred pred)
	{
		for (auto& c : str)
			if (pred(c))
				return true;
		return false;
	}

	template<typename String, typename Pred>
	bool any_not_pred(const String& str, Pred pred)
	{
		for (auto& c : str)
			if (!pred(c))
				return true;
		return false;
	}

	/* IS_EQUAL / IS_NOT_EQUAL / IS_LESS / ... */

#define STRING_ALGO_IS_COMP(NAME,COMP) \
	namespace detail { \
		template<typename Type> \
		struct NAME { \
			NAME (const Type& value) : _value(value) {} \
			template<typename Type2> bool operator()(const Type2& r) const { return r COMP _value; } \
		private: \
			const Type& _value; \
		}; \
	} \
	template<typename Type> detail:: NAME <Type> NAME (const Type& value) { return detail:: NAME <Type> (value); }

	STRING_ALGO_IS_COMP(is_equal,==)
	STRING_ALGO_IS_COMP(is_not_equal,!=)
	STRING_ALGO_IS_COMP(is_less,<)
	STRING_ALGO_IS_COMP(is_greater,>)
	STRING_ALGO_IS_COMP(is_lessequal,<=)
	STRING_ALGO_IS_COMP(is_greaterequal,>=)

	/* IS RANGE */

	template<typename Type>
	struct is_range_impl {
		is_range_impl(const Type& minval, const Type& maxval): _minval(minval), _maxval(maxval) {}
		template<typename Type2> bool operator()(const Type2& r) const { return _minval <= r && r <= _maxval; }
	private:
		const Type& _minval;
		const Type& _maxval;
	};
	template<typename Type>
	is_range_impl<Type> is_range(const Type& minval, const Type& maxval)
	{
		return is_range_impl<Type>(minval, maxval);
	}

	/* IS ANY OF */

	template<typename Container>
	struct is_any_of_impl {
		is_any_of_impl(const Container& container) : _cont(container) {}
		is_any_of_impl(Container&& container) : _cont(std::move(container)) {}
		bool operator()(const typename Container::value_type& r) const
		{
			return std::find(_cont.begin(),_cont.end(),r) != _cont.end();
		}
	private:
		Container _cont;
	};
	template<typename Container> is_any_of_impl<Container> is_any_of(const Container& cont) { return is_any_of_impl<Container>(cont); }
	template<typename Container> is_any_of_impl<Container> is_any_of(Container&& cont) { return is_any_of_impl<Container>(std::move(cont)); }

	is_any_of_impl<std::string> is_any_of(const char* str) { return is_any_of_impl<std::string>(std::string(str)); }
	is_any_of_impl<std::wstring> is_any_of(const wchar_t* str) { return is_any_of_impl<std::wstring>(std::wstring(str)); }

	/* ALL/ANY CHARACTER CLASS */

	struct all_character_class {
		all_character_class(std::ctype_base::mask mask, const std::locale& loc = std::locale())
			: _charfacet(std::use_facet<std::ctype<char>>(loc))
			, _wcharfacet(std::use_facet<std::ctype<wchar_t>>(loc))
			, _mask(mask)
		{}

		template<typename String>
		decltype(String().begin(),bool())
		/*bool*/ operator()(const String& str) const
		{
			return sa::all_pred(str, *this);
		}
		bool operator()(const char& c) const
		{
			return _charfacet.is(_mask,c);
		}
		bool operator()(const wchar_t& c) const
		{
			return _wcharfacet.is(_mask,c);
		}
	private:
		const std::ctype<char>& _charfacet;
		const std::ctype<wchar_t>& _wcharfacet;
		std::ctype_base::mask _mask;
	};

	struct any_character_class {
		any_character_class(std::ctype_base::mask mask, const std::locale& loc = std::locale())
			: _charfacet(std::use_facet<std::ctype<char>>(loc))
			, _wcharfacet(std::use_facet<std::ctype<wchar_t>>(loc))
			, _mask(mask)
		{}

		template<typename String>
		decltype(String().begin(),bool())
		/*bool*/ operator()(const String& str) const
		{
			return sa::any_pred(str, *this);
		}
		bool operator()(const char& c) const
		{
			return _charfacet.is(_mask,c);
		}
		bool operator()(const wchar_t& c) const
		{
			return _wcharfacet.is(_mask,c);
		}
	private:
		const std::ctype<char>& _charfacet;
		const std::ctype<wchar_t>& _wcharfacet;
		std::ctype_base::mask _mask;
	};

	/* MAKE PREDICATE IS SPACE/PRINT/CNTRL/UPPER/LOWER/ALPHA/DIGIT/PUNCT/XDIGIT/BLANK/ALNUM/GRAPH */

#define STRING_ALGO_ALL_CLASS(NAME,MASK) \
	all_character_class NAME (const std::locale& loc = std::locale()) \
	{ \
		return all_character_class( MASK , loc); \
	} \
	template<typename String> bool NAME (const String& str, const std::locale& loc = std::locale()) { return NAME (loc)(str); }

#define STRING_ALGO_ANY_CLASS(NAME,MASK) \
	any_character_class NAME (const std::locale& loc = std::locale()) \
	{ \
		return any_character_class( MASK , loc); \
	} \
	template<typename String> bool NAME (const String& str, const std::locale& loc = std::locale()) { return NAME (loc)(str); }

	STRING_ALGO_ALL_CLASS(is_space,std::ctype_base::space)
	STRING_ALGO_ALL_CLASS(is_print,std::ctype_base::print)
	STRING_ALGO_ALL_CLASS(is_cntrl,std::ctype_base::cntrl)
	STRING_ALGO_ALL_CLASS(is_upper,std::ctype_base::upper)
	STRING_ALGO_ALL_CLASS(is_lower,std::ctype_base::lower)
	STRING_ALGO_ALL_CLASS(is_alpha,std::ctype_base::alpha)
	STRING_ALGO_ALL_CLASS(is_digit,std::ctype_base::digit)
	STRING_ALGO_ALL_CLASS(is_punct,std::ctype_base::punct)
	STRING_ALGO_ALL_CLASS(is_xdigit,std::ctype_base::xdigit)
	STRING_ALGO_ALL_CLASS(is_alnum,std::ctype_base::alnum)
	STRING_ALGO_ALL_CLASS(is_graph,std::ctype_base::graph)

	STRING_ALGO_ALL_CLASS(all_space,std::ctype_base::space)
	STRING_ALGO_ALL_CLASS(all_print,std::ctype_base::print)
	STRING_ALGO_ALL_CLASS(all_cntrl,std::ctype_base::cntrl)
	STRING_ALGO_ALL_CLASS(all_upper,std::ctype_base::upper)
	STRING_ALGO_ALL_CLASS(all_lower,std::ctype_base::lower)
	STRING_ALGO_ALL_CLASS(all_alpha,std::ctype_base::alpha)
	STRING_ALGO_ALL_CLASS(all_digit,std::ctype_base::digit)
	STRING_ALGO_ALL_CLASS(all_punct,std::ctype_base::punct)
	STRING_ALGO_ALL_CLASS(all_xdigit,std::ctype_base::xdigit)
	STRING_ALGO_ALL_CLASS(all_alnum,std::ctype_base::alnum)
	STRING_ALGO_ALL_CLASS(all_graph,std::ctype_base::graph)

	STRING_ALGO_ANY_CLASS(any_space,std::ctype_base::space)
	STRING_ALGO_ANY_CLASS(any_print,std::ctype_base::print)
	STRING_ALGO_ANY_CLASS(any_cntrl,std::ctype_base::cntrl)
	STRING_ALGO_ANY_CLASS(any_upper,std::ctype_base::upper)
	STRING_ALGO_ANY_CLASS(any_lower,std::ctype_base::lower)
	STRING_ALGO_ANY_CLASS(any_alpha,std::ctype_base::alpha)
	STRING_ALGO_ANY_CLASS(any_digit,std::ctype_base::digit)
	STRING_ALGO_ANY_CLASS(any_punct,std::ctype_base::punct)
	STRING_ALGO_ANY_CLASS(any_xdigit,std::ctype_base::xdigit)
	STRING_ALGO_ANY_CLASS(any_alnum,std::ctype_base::alnum)
	STRING_ALGO_ANY_CLASS(any_graph,std::ctype_base::graph)

#ifdef STRING_ALGO_NEED_BLANK
	/*
	   std::isblank<CharT>(CharT,const std::locale&) is not properly defined in g++-4.8.5.
	   As isblank is not a common use-case, it is disabled by default here.
	   It can be enabled by the application when desired using '#define STRING_ALGO_NEED_BLANK'.
	*/
	STRING_ALGO_ALL_CLASS(is_blank,std::ctype_base::blank)
	STRING_ALGO_ALL_CLASS(all_blank,std::ctype_base::blank)
	STRING_ALGO_ANY_CLASS(any_blank,std::ctype_base::blank)
#endif

	/* TO LOWER / UPPER */

	template<typename String>
	void to_lower(String& str, const std::locale& loc = std::locale())
	{
		typedef typename String::value_type CharT;
		sa::modify(str, [&loc](CharT& c){ sa::to_lower(c, loc); });
	}

	template<typename String>
	String to_lower_copy(const String& str, const std::locale& loc = std::locale())
	{
		typedef typename String::value_type CharT;
		return sa::modify_copy(str, [&loc](CharT& c){ sa::to_lower(c, loc); });
	}

	template<> void to_lower<char>(char& c, const std::locale& loc) { c = std::tolower(c, loc); }
	template<> void to_lower<wchar_t>(wchar_t& c, const std::locale& loc) { c = std::tolower(c, loc); }
	template<> char to_lower_copy<char>(const char& c, const std::locale& loc) { return std::tolower(c, loc); }
	template<> wchar_t to_lower_copy<wchar_t>(const wchar_t& c, const std::locale& loc) { return std::tolower(c, loc); }

	template<typename String>
	void to_upper(String& str, const std::locale& loc = std::locale())
	{
		typedef typename String::value_type CharT;
		sa::modify(str, [&loc](CharT& c){ sa::to_upper(c, loc); });
	}

	template<typename String>
	String to_upper_copy(const String& str, const std::locale& loc = std::locale())
	{
		typedef typename String::value_type CharT;
		return sa::modify_copy(str, [&loc](CharT& c){ sa::to_upper(c, loc); });
	}

	template<> void to_upper<char>(char& c, const std::locale& loc) { c = std::toupper(c, loc); }
	template<> void to_upper<wchar_t>(wchar_t& c, const std::locale& loc) { c = std::toupper(c, loc); }
	template<> char to_upper_copy<char>(const char& c, const std::locale& loc) { return std::toupper(c, loc); }
	template<> wchar_t to_upper_copy<wchar_t>(const wchar_t& c, const std::locale& loc) { return std::toupper(c, loc); }

	/* LEFT / MID / RIGHT COPY */

	template<typename String>
	String left_copy(const String& str, std::size_t count)
	{
		if (count > str.size())
			count = str.size();
		return String(str.begin(), advance_copy(str.begin(),count));
	}

	template<typename String>
	String mid_copy(const String& str, std::size_t pos, std::size_t count = std::string::npos)
	{
		if (pos > str.size())
			pos = str.size();
		if (count > str.size() - pos)
			count = str.size() - pos;
		return String(advance_copy(str.begin(),pos),advance_copy(str.begin(),pos+count));
	}

	template<typename String>
	String right_copy(const String& str, std::size_t count)
	{
		if (count > str.size())
			count = str.size();
		return String(advance_copy(str.begin(),str.size()-count),str.end());
	}

	/* FIND FIRST ELEM */

	template<typename String, typename StringIterator>
	StringIterator find(String& str, const typename String::value_type& c, StringIterator pos)
	{
		auto end = str.end();
		for (; pos != end && *pos != c; ++pos)
			;
		return pos;
	}
	template<typename String>
	auto find(String& str, const typename String::value_type& c) -> decltype(str.begin())
	{
		return sa::find(str, c, str.begin());
	}

	/* IFIND FIRST ELEM */

	template<typename String, typename StringIterator>
	StringIterator ifind(String& str, const typename String::value_type& c, StringIterator pos, const std::locale& loc = std::locale())
	{
		auto clower = sa::to_lower_copy(c, loc);
		auto end = str.end();
		for (; pos != end && sa::to_lower_copy(*pos, loc) != clower; ++pos)
			;
		return pos;
	}
	template<typename String>
	auto ifind(String& str, const typename String::value_type& c, const std::locale& loc = std::locale()) -> decltype(str.begin())
	{
		return sa::ifind(str, c, str.begin(), loc);
	}

	/* FIND FIRST SUBSTRING */

	template<typename String, typename String2, typename StringIterator>
	decltype(String().begin(),String2().begin(),StringIterator())
	/*StringIterator*/ find(String& str, const String2& substr, StringIterator pos)
	{
		if (substr.empty())
			return pos;
		auto subpos = substr.begin();
		while ((pos = sa::find(str, *subpos, pos)) != str.end())
		{
			auto pos2 = pos;
			auto subpos2 = subpos;
			++pos2; ++subpos2;
			for (; pos2 != str.end() && subpos2 != substr.end() && *pos2 == *subpos2; ++pos2, ++subpos2)
				;
			if (subpos2  == substr.end())
				return pos;
			++pos;
		}
		return pos;
	}
	template<typename String, typename String2>
	auto find(String& str, const String2& substr) -> decltype(substr.begin(),str.begin())
	{
		return sa::find(str, substr, str.begin());
	}
	template<typename String, typename StringIterator>
	StringIterator find(String& str, const char* csubstr, StringIterator pos)
	{
		return sa::find(str, String(csubstr), str.begin());
	}
	template<typename String>
	auto find(String& str, const char* substr) -> decltype(str.begin())
	{
		return sa::find(str, String(substr), str.begin());
	}
	template<typename String, typename StringIterator>
	StringIterator find(String& str, const wchar_t* csubstr, StringIterator pos)
	{
		return sa::find(str, String(csubstr), str.begin());
	}
	template<typename String>
	auto find(String& str, const wchar_t* substr) -> decltype(str.begin())
	{
		return sa::find(str, String(substr), str.begin());
	}

	/* IFIND FIRST SUBSTRING */

	template<typename String, typename String2, typename StringIterator>
	decltype(String().begin(),String2().begin(),StringIterator())
	/*StringIterator*/ ifind(String& str, const String2& substr, StringIterator pos, const std::locale& loc = std::locale())
	{
		if (substr.empty())
			return pos;
		String substrlower = sa::to_lower_copy(substr, loc);
		auto subpos = substrlower.begin();
		while ((pos = sa::ifind(str, *subpos, pos)) != str.end())
		{
			auto pos2 = pos;
			auto subpos2 = subpos;
			++pos2; ++subpos2;
			for (; pos2 != str.end() && subpos2 != substrlower.end() && sa::to_lower_copy(*pos2, loc) == *subpos2; ++pos2, ++subpos2)
				;
			if (subpos2  == substrlower.end())
				return pos;
			++pos;
		}
		return pos;
	}
	template<typename String, typename String2>
	auto ifind(String& str, const String2& substr, const std::locale& loc = std::locale()) -> decltype(substr.begin(),str.begin())
	{
		return sa::ifind(str, substr, str.begin(), loc);
	}
	template<typename String, typename StringIterator>
	StringIterator ifind(String& str, const char* substr, StringIterator pos, const std::locale& loc = std::locale())
	{
		return sa::ifind(str, String(substr), pos, loc);
	}
	template<typename String>
	auto ifind(String& str, const char* substr, const std::locale& loc = std::locale()) -> decltype(str.begin())
	{
		return sa::ifind(str, String(substr), str.begin(), loc);
	}
	template<typename String, typename StringIterator>
	StringIterator ifind(String& str, const wchar_t* substr, StringIterator pos, const std::locale& loc = std::locale())
	{
		return sa::ifind(str, String(substr), pos, loc);
	}
	template<typename String>
	auto ifind(String& str, const wchar_t* substr, const std::locale& loc = std::locale()) -> decltype(str.begin())
	{
		return sa::ifind(str, String(substr), str.begin(), loc);
	}

	/* FIND LAST ELEM */

	template<typename String, typename StringIterator>
	StringIterator find_last(String& str, const typename String::value_type& c, StringIterator pos)
	{
		if (pos != str.end() && *pos == c)
			return pos;
		auto beg = str.begin();
		while (pos != beg && *--pos != c)
			;
		if (pos == beg && *pos != c)
			return str.end();
		return pos;
	}
	template<typename String>
	auto find_last(String& str, const typename String::value_type& c) -> decltype(str.begin())
	{
		return sa::find_last(str, c, str.end());
	}

	/* IFIND LAST ELEM */

	template<typename String, typename StringIterator>
	StringIterator ifind_last(String& str, const typename String::value_type& c, StringIterator pos, const std::locale& loc = std::locale())
	{
		auto clower = sa::to_lower_copy(c, loc);
		if (pos != str.end() && sa::to_lower_copy(*pos, loc) == clower)
			return pos;
		auto beg = str.begin();
		while (pos != beg && sa::to_lower_copy(*--pos, loc) != clower)
			;
		if (pos == beg && sa::to_lower_copy(*pos, loc) != clower)
			return str.end();
		return pos;
	}
	template<typename String>
	auto ifind_last(String& str, const typename String::value_type& c, const std::locale& loc = std::locale()) -> decltype(str.begin())
	{
		return sa::ifind_last(str, c, str.end(), loc);
	}

	/* FIND LAST SUBSTRING */

	template<typename String, typename String2, typename StringIterator>
	decltype(String().begin(),String2().begin(),StringIterator())
	/*StringIterator*/ find_last(String& str, const String2& substr, StringIterator pos)
	{
		if (substr.empty())
			return pos;
		auto subpos = substr.begin();
		while ((pos = sa::find_last(str, *subpos, pos)) != str.end())
		{
			auto pos2 = pos;
			auto subpos2 = subpos;
			++pos2; ++subpos2;
			for (; pos2 != str.end() && subpos2 != substr.end() && *pos2 == *subpos2; ++pos2, ++subpos2)
				;
			if (subpos2  == substr.end())
				return pos;
			if (pos == str.begin())
				return str.end();
			--pos;
		}
		return str.end();
	}
	template<typename String, typename String2>
	auto find_last(String& str, const String2& substr) -> decltype(substr.begin(),str.begin())
	{
		return sa::find_last(str, substr, str.end());
	}
	template<typename String, typename StringIterator>
	StringIterator find_last(String& str, const char* substr, StringIterator pos)
	{
		return sa::find_last(str, String(substr), pos);
	}
	template<typename String>
	auto find_last(String& str, const char* substr) -> decltype(str.begin())
	{
		return sa::find_last(str, String(substr), str.end());
	}
	template<typename String, typename StringIterator>
	StringIterator find_last(String& str, const wchar_t* substr, StringIterator pos)
	{
		return sa::find_last(str, String(substr), pos);
	}
	template<typename String>
	auto find_last(String& str, const wchar_t* substr) -> decltype(str.begin())
	{
		return sa::find_last(str, String(substr), str.end());
	}

	/* IFIND LAST SUBSTRING */

	template<typename String, typename String2, typename StringIterator>
	decltype(String().begin(),String2().begin(),StringIterator())
	/*StringIterator*/ ifind_last(String& str, const String2& substr, StringIterator pos, const std::locale& loc = std::locale())
	{
		if (substr.empty())
			return pos;
		String substrlower = sa::to_lower_copy(substr, loc);
		auto subpos = substrlower.begin();
		while ((pos = ifind_last(str, *subpos, pos)) != str.end())
		{
			auto pos2 = pos;
			auto subpos2 = subpos;
			++pos2; ++subpos2;
			for (; pos2 != str.end() && subpos2 != substrlower.end() && sa::to_lower_copy(*pos2, loc) == *subpos2; ++pos2, ++subpos2)
				;
			if (subpos2  == substrlower.end())
				return pos;
			if (pos == str.begin())
				return str.end();
			--pos;
		}
		return str.end();
	}
	template<typename String, typename String2>
	auto ifind_last(String& str, const String2& substr, const std::locale& loc = std::locale()) -> decltype(substr.begin(),str.begin())
	{
		return sa::ifind_last(str, substr, str.end(), loc);
	}
	template<typename String, typename StringIterator>
	StringIterator ifind_last(String& str, const char* substr, StringIterator pos, const std::locale& loc = std::locale())
	{
		return sa::ifind_last(str, String(substr), pos, loc);
	}
	template<typename String>
	auto ifind_last(String& str, const char* substr, const std::locale& loc = std::locale()) -> decltype(str.begin())
	{
		return sa::ifind_last(str, String(substr), str.end(), loc);
	}
	template<typename String, typename StringIterator>
	StringIterator ifind_last(String& str, const wchar_t* substr, StringIterator pos, const std::locale& loc = std::locale())
	{
		return sa::ifind_last(str, String(substr), pos, loc);
	}
	template<typename String>
	auto ifind_last(String& str, const wchar_t* substr, const std::locale& loc = std::locale()) -> decltype(str.begin())
	{
		return sa::ifind_last(str, String(substr), str.end(), loc);
	}

	/* FIND FIRST PRED */

	template<typename String, typename Pred, typename StringIterator>
	StringIterator find_pred(String& str, Pred pred, StringIterator pos)
	{
		auto end = str.end();
		for (; pos != end && !pred(*pos); ++pos)
			;
		return pos;
	}
	template<typename String, typename Pred>
	auto find_pred(String& str, Pred pred) -> decltype(str.begin())
	{
		return sa::find_pred(str, pred, str.begin());
	}

	/* FIND FIRST NOT PRED */

	template<typename String, typename Pred, typename StringIterator>
	StringIterator find_not_pred(String& str, Pred pred, StringIterator pos)
	{
		auto end = str.end();
		for (; pos != end && pred(*pos); ++pos)
			;
		return pos;
	}
	template<typename String, typename Pred>
	auto find_not_pred(String& str, Pred pred) -> decltype(str.begin())
	{
		return sa::find_not_pred(str, pred, str.begin());
	}

	/* FIND LAST PRED */

	template<typename String, typename Pred, typename StringIterator>
	StringIterator find_last_pred(String& str, Pred pred, StringIterator pos)
	{
		if (pos != str.end() && pred(*pos))
			return pos;
		auto beg = str.begin();
		while (pos != beg && !pred(*--pos))
			;
		if (pos == beg && !pred(*pos))
			return str.end();
		return pos;
	}
	template<typename String, typename Pred>
	auto find_last_pred(String& str, Pred pred) -> decltype(str.begin())
	{
		return sa::find_last_pred(str, pred, str.end());
	}

	/* FIND LAST NOT PRED */

	template<typename String, typename Pred, typename StringIterator>
	StringIterator find_last_not_pred(String& str, Pred pred, StringIterator pos)
	{
		if (pos != str.end() && pred(*pos))
			return pos;
		auto beg = str.begin();
		while (pos != beg && pred(*--pos))
			;
		if (pos == beg && pred(*pos))
			return str.end();
		return pos;
	}
	template<typename String, typename Pred>
	auto find_last_not_pred(String& str, Pred pred) -> decltype(str.begin())
	{
		return sa::find_last_not_pred(str, pred, str.end());
	}

	/* STARTS_WITH */

	template<typename String>
	bool starts_with(const String& str, const String& pre)
	{
		auto it1 = str.begin(), it1end = str.end();
		auto it2 = pre.begin(), it2end = pre.end();
		for (; it1 != it1end && it2 != it2end && (*it1) == (*it2);  ++it1,++it2)
			;
		return it2 == it2end;
	}
	template<typename String>
	bool starts_with(const String& str, const char* pre)
	{
		return sa::starts_with(str, String(pre));
	}
	template<typename String>
	bool starts_with(const String& str, const wchar_t* pre)
	{
		return sa::starts_with(str, String(pre));
	}

	template<typename String>
	bool istarts_with(const String& str, const String& pre, const std::locale& loc = std::locale())
	{
		auto it1 = str.begin(), it1end = str.end();
		auto it2 = pre.begin(), it2end = pre.end();
		for (; it1 != it1end && it2 != it2end && sa::to_lower_copy(*it1,loc) == sa::to_lower_copy(*it2,loc); ++it1,++it2)
			;
		return it2 == it2end;
	}
	template<typename String>
	bool istarts_with(const String& str, const char* pre)
	{
		return sa::istarts_with(str, String(pre));
	}
	template<typename String>
	bool istarts_with(const String& str, const wchar_t* pre)
	{
		return sa::istarts_with(str, String(pre));
	}

	/* ENDS_WITH */

	template<typename String>
	bool ends_with(const String& str, const String& pre)
	{
		if (pre.size() > str.size())
			return false;
		auto it1 = sa::advance_copy(str.begin(),str.size()-pre.size()), it1end = str.end();
		auto it2 = pre.begin(), it2end = pre.end();
		for (; it1 != it1end && it2 != it2end && (*it1) == (*it2);  ++it1,++it2)
			;
		return it2 == it2end;
	}
	template<typename String>
	bool ends_with(const String& str, const char* pre)
	{
		return sa::ends_with(str, String(pre));
	}
	template<typename String>
	bool ends_with(const String& str, const wchar_t* pre)
	{
		return sa::ends_with(str, String(pre));
	}

	template<typename String>
	bool iends_with(const String& str, const String& pre, const std::locale& loc = std::locale())
	{
		if (pre.size() > str.size())
			return false;
		auto it1 = sa::advance_copy(str.begin(),str.size()-pre.size()), it1end = str.end();
		auto it2 = pre.begin(), it2end = pre.end();
		for (; it1 != it1end && it2 != it2end && sa::to_lower_copy(*it1,loc) == sa::to_lower_copy(*it2,loc); ++it1,++it2)
			;
		return it2 == it2end;
	}
	template<typename String>
	bool iends_with(const String& str, const char* pre)
	{
		return sa::iends_with(str, String(pre));
	}
	template<typename String>
	bool iends_with(const String& str, const wchar_t* pre)
	{
		return sa::iends_with(str, String(pre));
	}

	/* CONTAINS */

	template<typename String>
	bool contains(const String& str, const String& pre)
	{
		return sa::find(str, pre) != str.end();
	}
	template<typename String>
	bool contains(const String& str, const char* pre)
	{
		return sa::contains(str, String(pre));
	}
	template<typename String>
	bool contains(const String& str, const wchar_t* pre)
	{
		return sa::contains(str, String(pre));
	}

	template<typename String>
	bool icontains(const String& str, const String& pre, const std::locale& loc = std::locale())
	{
		return sa::ifind(str, pre, loc) != str.end();
	}
	template<typename String>
	bool icontains(const String& str, const char* pre)
	{
		return sa::icontains(str, String(pre));
	}
	template<typename String>
	bool icontains(const String& str, const wchar_t* pre)
	{
		return sa::icontains(str, String(pre));
	}

	/* EQUALS */

	template<typename String>
	bool equals(const String& str, const String& pre)
	{
		return str == pre;
	}
	template<typename String>
	bool equals(const String& str, const char* pre)
	{
		return sa::equals(str, String(pre));
	}
	template<typename String>
	bool equals(const String& str, const wchar_t* pre)
	{
		return sa::equals(str, String(pre));
	}

	template<typename String>
	bool iequals(const String& str, const String& pre, const std::locale& loc = std::locale())
	{
		return str.size() == pre.size() && sa::istarts_with(str, pre, loc);
	}
	template<typename String>
	bool iequals(const String& str, const char* pre)
	{
		return sa::iequals(str, String(pre));
	}
	template<typename String>
	bool iequals(const String& str, const wchar_t* pre)
	{
		return sa::iequals(str, String(pre));
	}

	/* LEX_LESS */

	template<typename String>
	bool lex_less(const String& str, const String& pre)
	{
		auto it1 = str.begin(), it1end = str.end();
		auto it2 = pre.begin(), it2end = pre.end();
		for (; it1 != it1end && it2 != it2end && (*it1) == (*it2);  ++it1,++it2)
			;
		if (it1 == it1end)
			return it2 != it2end;
		if (it2 == it2end)
			return false;
		return (*it1) < (*it2);
	}

	template<typename String>
	bool ilex_less(const String& str, const String& pre, const std::locale& loc = std::locale())
	{
		auto it1 = str.begin(), it1end = str.end();
		auto it2 = pre.begin(), it2end = pre.end();
		for (; it1 != it1end && it2 != it2end && sa::to_lower_copy(*it1,loc) == sa::to_lower_copy(*it2,loc);  ++it1,++it2)
			;
		if (it1 == it1end)
			return it2 != it2end;
		if (it2 == it2end)
			return false;
		return sa::to_lower_copy(*it1,loc) < sa::to_lower_copy(*it2,loc);
	}

	/* SPLIT */

	template<typename String, typename Pred>
	std::vector<String> split_pred(const String& str, Pred pred)
	{
		std::vector<String> ret;
		auto itbeg = str.begin(), it = itbeg, itend = str.end();
		while ((it = sa::find_pred(str, pred, itbeg)) != itend)
		{
			ret.emplace_back(itbeg, it);
			itbeg = ++it;
		}
		ret.emplace_back(itbeg, str.end());
		return ret;
	}

	template<typename Container, typename String, typename Pred>
	void split(Container& cont, const String& str, Pred pred)
	{
		cont = Container();
		auto itbeg = str.begin(), it = itbeg, itend = str.end();
		while ((it = sa::find_pred(str, pred, itbeg)) != itend)
		{
			cont.emplace_back(itbeg, it);
			itbeg = ++it;
		}
		cont.emplace_back(itbeg, str.end());
	}

	template<typename String>
	std::vector<String> split(const String& str, const typename String::value_type& delim)
	{
		return sa::split_pred(str, is_equal(delim));
	}

	template<typename Container, typename String>
	void split(Container& cont, const String& str, const typename String::value_type& delim)
	{
		sa::split_pred(cont, str, is_equal(delim));
	}

	template<typename String>
	std::vector<String> split(const String& str, const String& delims)
	{
		return sa::split_pred(str, is_any_of(delims));
	}

	template<typename Container, typename String>
	void split(Container& cont, const String& str, const String& delims)
	{
		sa::split_pred(cont, str, is_any_of(delims));
	}

	/* APPEND */

	template<typename String, typename Iterator>
	void append(String& dest, Iterator first, Iterator last)
	{
		std::copy(first, last, std::back_inserter(dest));
	}
	template<typename String>
	void append(String& dest, const typename String::value_type& c)
	{
		*std::back_inserter(dest) = c;
	}
	template<typename String, typename String2>
	decltype(String2().begin(), void()) // SFINAE
	/*void*/ append(String& dest, const String2& suffix)
	{
		std::copy(suffix.begin(), suffix.end(), std::back_inserter(dest));
	}
	template<typename String>
	void append(String& dest, const char* src)
	{
		sa::append(dest,String(src));
	}
	template<typename String>
	void append(String& dest, const wchar_t* src)
	{
		sa::append(dest,String(src));
	}

	/* JOIN */

	template<typename Container, typename Separator = typename Container::value_type>
	auto join(const Container& container, const Separator& separator = Separator()) -> typename Container::value_type
	{
		typename Container::value_type ret;
		auto it = container.begin(), itend = container.end();
		if (it == itend)
			return ret;
		sa::append(ret, *it);
		for (++it; it != itend; ++it)
		{
			sa::append(ret, separator);
			sa::append(ret, *it);
		}
		return ret;
	}

	template<typename Container, typename Pred, typename Separator = typename Container::value_type>
	auto join_pred(const Container& container, Pred pred, const Separator& separator = Separator()) -> typename Container::value_type
	{
		typename Container::value_type ret;
		auto it = container.begin(), itend = container.end();
		for (; it != itend && !pred(*it); ++it)
			;
		if (it == itend)
			return ret;
		sa::append(ret, *it);
		for (++it; it != itend; ++it)
		{
			if (pred(*it))
			{
				sa::append(ret, separator);
				sa::append(ret, *it);
			}
		}
		return ret;
	}

	/* TRIM LEFT */

	template<typename String, typename Pred>
	void trim_left_pred(String& str, Pred pred)
	{
		auto pos = sa::find_not_pred(str, pred);
		str.erase(str.begin(), pos);
	}

	template<typename String, typename Pred>
	String trim_left_copy_pred(const String& str, Pred pred)
	{
		auto pos = sa::find_not_pred(str, pred);
		return String(pos, str.end());
	}

	template<typename String>
	void trim_left(String& str, const String& trim_chars)
	{
		sa::trim_left_pred(str, sa::is_any_of(trim_chars));
	}
	template<typename String>
	String trim_left_copy(const String& str, const String& trim_chars)
	{
		return sa::trim_left_copy_pred(str, sa::is_any_of(trim_chars));
	}
	template<typename String>
	void trim_left(String& str, const char* trim_chars)
	{
		sa::trim_left_pred(str, sa::is_any_of(String(trim_chars)));
	}
	template<typename String>
	String trim_left_copy(const String& str, const char* trim_chars)
	{
		return sa::trim_left_copy_pred(str, sa::is_any_of(String(trim_chars)));
	}
	template<typename String>
	void trim_left(String& str, const wchar_t* trim_chars)
	{
		sa::trim_left_pred(str, sa::is_any_of(String(trim_chars)));
	}
	template<typename String>
	String trim_left_copy(const String& str, const wchar_t* trim_chars)
	{
		return sa::trim_left_copy_pred(str, sa::is_any_of(String(trim_chars)));
	}
	template<typename String>
	void trim_left(String& str, char trim_char)
	{
		sa::trim_left_pred(str, sa::is_equal(trim_char));
	}
	template<typename String>
	String trim_left_copy(const String& str, char trim_char)
	{
		return sa::trim_left_copy_pred(str, sa::is_equal(trim_char));
	}
	template<typename String>
	void trim_left(String& str, wchar_t trim_char)
	{
		sa::trim_left_pred(str, sa::is_equal(trim_char));
	}
	template<typename String>
	String trim_left_copy(const String& str, wchar_t trim_char)
	{
		return sa::trim_left_copy_pred(str, sa::is_equal(trim_char));
	}

	template<typename String>
	void trim_left(String& str, const std::locale& loc = std::locale())
	{
		sa::trim_left_pred(str, sa::is_space(loc));
	}

	template<typename String>
	String trim_left_copy(const String& str, const std::locale& loc = std::locale())
	{
		return sa::trim_left_copy_pred(str, sa::is_space(loc));
	}

	/* TRIM RIGHT */

	template<typename String, typename Pred>
	void trim_right_pred(String& str, Pred pred)
	{
		auto pos = sa::find_last_not_pred(str, pred);
		if (pos == str.end())
			str.erase(str.begin(), str.end());
		else
			str.erase(++pos, str.end());
	}

	template<typename String, typename Pred>
	String trim_right_copy_pred(const String& str, Pred pred)
	{
		auto pos = sa::find_last_not_pred(str, pred);
		if (pos == str.end())
			return String();
		else
			return String(str.begin(), ++pos);
	}

	template<typename String>
	void trim_right(String& str, const String& trim_chars)
	{
		sa::trim_right_pred(str, sa::is_any_of(trim_chars));
	}
	template<typename String>
	String trim_right_copy(const String& str, const String& trim_chars)
	{
		return sa::trim_right_copy_pred(str, sa::is_any_of(trim_chars));
	}
	template<typename String>
	void trim_right(String& str, const char* trim_chars)
	{
		sa::trim_right_pred(str, sa::is_any_of(String(trim_chars)));
	}
	template<typename String>
	String trim_right_copy(const String& str, const char* trim_chars)
	{
		return sa::trim_right_copy_pred(str, sa::is_any_of(String(trim_chars)));
	}
	template<typename String>
	void trim_right(String& str, const wchar_t* trim_chars)
	{
		sa::trim_right_pred(str, sa::is_any_of(String(trim_chars)));
	}
	template<typename String>
	String trim_right_copy(const String& str, const wchar_t* trim_chars)
	{
		return sa::trim_right_copy_pred(str, sa::is_any_of(String(trim_chars)));
	}
	template<typename String>
	void trim_right(String& str, char trim_char)
	{
		sa::trim_right_pred(str, sa::is_equal(trim_char));
	}
	template<typename String>
	String trim_right_copy(const String& str, char trim_char)
	{
		return sa::trim_right_copy_pred(str, sa::is_equal(trim_char));
	}
	template<typename String>
	void trim_right(String& str, wchar_t trim_char)
	{
		sa::trim_right_pred(str, sa::is_equal(trim_char));
	}
	template<typename String>
	String trim_right_copy(const String& str, wchar_t trim_char)
	{
		return sa::trim_right_copy_pred(str, sa::is_equal(trim_char));
	}

	template<typename String>
	void trim_right(String& str, const std::locale& loc = std::locale())
	{
		sa::trim_right_pred(str, sa::is_space(loc));
	}

	template<typename String>
	String trim_right_copy(const String& str, const std::locale& loc = std::locale())
	{
		return sa::trim_right_copy_pred(str, sa::is_space(loc));
	}

	/* TRIM */

	template<typename String, typename Pred>
	void trim_pred(String& str, Pred pred)
	{
		sa::trim_right_pred(str,pred);
		sa::trim_left_pred(str,pred);
	}

	template<typename String, typename Pred>
	String trim_copy_pred(const String& str, Pred pred)
	{
		auto posl = sa::find_not_pred(str, pred);
		auto posr = sa::find_last_not_pred(str, pred);
		if (posr == str.end())
			return String();
		else
			return String(posl, ++posr);
	}

	template<typename String>
	void trim(String& str, const String& trim_chars)
	{
		sa::trim_pred(str, sa::is_any_of(trim_chars));
	}
	template<typename String>
	String trim_copy(const String& str, const String& trim_chars)
	{
		return sa::trim_copy_pred(str, sa::is_any_of(trim_chars));
	}
	template<typename String>
	void trim(String& str, const char* trim_chars)
	{
		sa::trim_pred(str, sa::is_any_of(String(trim_chars)));
	}
	template<typename String>
	String trim_copy(const String& str, const char* trim_chars)
	{
		return sa::trim_copy_pred(str, sa::is_any_of(String(trim_chars)));
	}
	template<typename String>
	void trim(String& str, const wchar_t* trim_chars)
	{
		sa::trim_pred(str, sa::is_any_of(String(trim_chars)));
	}
	template<typename String>
	String trim_copy(const String& str, const wchar_t* trim_chars)
	{
		return sa::trim_copy_pred(str, sa::is_any_of(String(trim_chars)));
	}
	template<typename String>
	void trim(String& str, char trim_char)
	{
		sa::trim_pred(str, sa::is_equal(trim_char));
	}
	template<typename String>
	String trim_copy(const String& str, char trim_char)
	{
		return sa::trim_copy_pred(str, sa::is_equal(trim_char));
	}
	template<typename String>
	void trim(String& str, wchar_t trim_char)
	{
		sa::trim_pred(str, sa::is_equal(trim_char));
	}
	template<typename String>
	String trim_copy(const String& str, wchar_t trim_char)
	{
		return sa::trim_copy_pred(str, sa::is_equal(trim_char));
	}

	template<typename String>
	void trim(String& str, const std::locale& loc = std::locale())
	{
		sa::trim_pred(str, sa::is_space(loc));
	}

	template<typename String>
	String trim_copy(const String& str, const std::locale& loc = std::locale())
	{
		return sa::trim_copy_pred(str, sa::is_space(loc));
	}

	/* REPLACE */

	template<typename String>
	void replace(String& str, std::size_t pos, std::size_t count, const String& replace)
	{
		if (pos > str.size())
			pos = str.size();
		if (count > str.size() - pos)
			count = str.size() - pos;
		if (replace.size() > count)
		{
			std::fill_n(std::back_inserter(str), replace.size()-count, typename String::value_type());
			std::move_backward(
				sa::advance_copy(str.begin(), pos+count),
				sa::advance_copy(str.begin(), str.size()-(replace.size()-count)),
				str.end());
		}
		std::copy(replace.begin(), replace.end(), sa::advance_copy(str.begin(),pos));
		if (replace.size() < count)
		{
			auto end = std::move(
				sa::advance_copy(str.begin(),pos+count),
				str.end(),
				sa::advance_copy(str.begin(),pos+replace.size()));
			str.erase(end, str.end());
		}
	}

	template<typename String>
	String replace_copy(const String& str, std::size_t pos, std::size_t count, const String& replace)
	{
		if (pos > str.size())
			pos = str.size();
		if (count > str.size() - pos)
			count = str.size() - pos;
		String ret(str.begin(), sa::advance_copy(str.begin(),pos));
		sa::append(ret, replace);
		sa::append(ret, sa::advance_copy(str.begin(),pos+count), str.end());
		return ret;
	}

	/* REPLACE HEAD / TAIL */

	template<typename String>
	void replace_head(String& str, std::size_t count, const String& replace)
	{
		sa::replace(str, 0, count, replace);
	}

	template<typename String>
	String replace_head_copy(const String& str, std::size_t count, const String& replace)
	{
		return sa::replace_copy(str, 0, count, replace);
	}

	template<typename String>
	void replace_tail(String& str, std::size_t count, const String& replace)
	{
		sa::replace(str, count < str.size() ? str.size()-count : 0, count, replace);
	}

	template<typename String>
	String replace_tail_copy(const String& str, std::size_t count, const String& replace)
	{
		return sa::replace_copy(str, count < str.size() ? str.size()-count : 0, count, replace);
	}

	/* INSERT */

	template<typename String>
	void insert(String& dest, std::size_t pos, const String& src)
	{
		if (pos >= dest.size() || src.empty())
		{
			sa::append(dest, src);
			return;
		}
		std::fill_n(std::back_inserter(dest), src.size(), typename String::value_type());
		std::move_backward(sa::advance_copy(dest.begin(),pos), sa::advance_copy(dest.begin(),dest.size()-src.size()), dest.end());
		std::copy(src.begin(), src.end(), sa::advance_copy(dest.begin(),pos));
	}

	template<typename String>
	String insert_copy(const String& dest, std::size_t pos, const String& src)
	{
		return sa::replace_copy(dest, pos, 0, src);
	}

	/* ERASE */

	template<typename String>
	void erase(String& str, std::size_t pos, std::size_t count = std::string::npos)
	{
		if (pos > str.size())
			pos = str.size();
		if (count > str.size()-pos)
			count = str.size()-pos;
		auto end = std::move(sa::advance_copy(str.begin(),pos+count), str.end(), sa::advance_copy(str.begin(),pos));
		str.erase(end, str.end());
	}

	template<typename String>
	String erase_copy(const String& str, std::size_t pos, std::size_t count = std::string::npos)
	{
		return sa::replace_copy(str, pos, count, String());
	}

	/* ERASE PRED */

	template<typename String, typename Pred>
	void erase_pred(String& str, Pred pred)
	{
		auto pos = str.begin();
		while (pos != str.end())
		{
			if (pred(*pos))
				pos = str.erase(pos);
			else
				++pos;
		}
	}

	template<typename String, typename Pred>
	String erase_pred_copy(const String& str, Pred pred)
	{
		String ret;
		auto bi = std::back_inserter(ret);
		for (auto& c : str)
			if (!pred(c))
				*bi = c;
		return ret;
	}

	/* REPLACE FIRST */

	template<typename String>
	void replace_first(String& dest, const String& search, const String& replace)
	{
		auto pos = sa::find(dest, search);
		if (pos != dest.end())
			sa::replace(dest, std::distance(dest.begin(), pos), search.size(), replace);
	}

	template<typename String>
	String replace_first_copy(const String& dest, const String& search, const String& replace)
	{
		auto pos = sa::find(dest, search);
		if (pos == dest.end())
			return dest;
		return sa::replace_copy(dest, std::distance(dest.begin(), pos), search.size(), replace);
	}

	template<typename String>
	void ireplace_first(String& dest, const String& search, const String& replace, const std::locale& loc = std::locale())
	{
		auto pos = sa::ifind(dest, search, loc);
		if (pos != dest.end())
			sa::replace(dest, std::distance(dest.begin(), pos), search.size(), replace);
	}

	template<typename String>
	String ireplace_first_copy(const String& dest, const String& search, const String& replace, const std::locale& loc = std::locale())
	{
		auto pos = sa::ifind(dest, search, loc);
		if (pos == dest.end())
			return dest;
		return sa::replace_copy(dest, std::distance(dest.begin(), pos), search.size(), replace);
	}

	/* ERASE FIRST */

	template<typename String>
	void erase_first(String& dest, const String& search)
	{
		auto pos = sa::find(dest, search);
		if (pos != dest.end())
			sa::erase(dest, std::distance(dest.begin(), pos), search.size());
	}

	template<typename String>
	String erase_first_copy(const String& dest, const String& search)
	{
		auto pos = sa::find(dest, search);
		if (pos == dest.end())
			return dest;
		return sa::erase_copy(dest, std::distance(dest.begin(), pos), search.size());
	}

	template<typename String>
	void ierase_first(String& dest, const String& search, const std::locale& loc = std::locale())
	{
		auto pos = sa::ifind(dest, search, loc);
		if (pos != dest.end())
			sa::erase(dest, std::distance(dest.begin(), pos), search.size());
	}

	template<typename String>
	String ierase_first_copy(const String& dest, const String& search, const std::locale& loc = std::locale())
	{
		auto pos = sa::ifind(dest, search, loc);
		if (pos == dest.end())
			return dest;
		return erase_copy(dest, std::distance(dest.begin(), pos), search.size());
	}

	/* REPLACE LAST */

	template<typename String>
	void replace_last(String& dest, const String& search, const String& replace)
	{
		auto pos = sa::find_last(dest, search);
		if (pos != dest.end())
			sa::replace(dest, std::distance(dest.begin(), pos), search.size(), replace);
	}

	template<typename String>
	String replace_last_copy(const String& dest, const String& search, const String& replace)
	{
		auto pos = sa::find_last(dest, search);
		if (pos == dest.end())
			return dest;
		return sa::replace_copy(dest, std::distance(dest.begin(), pos), search.size(), replace);
	}

	template<typename String>
	void ireplace_last(String& dest, const String& search, const String& replace, const std::locale& loc = std::locale())
	{
		auto pos = sa::ifind_last(dest, search, loc);
		if (pos != dest.end())
			sa::replace(dest, std::distance(dest.begin(), pos), search.size(), replace);
	}

	template<typename String>
	String ireplace_last_copy(const String& dest, const String& search, const String& replace, const std::locale& loc = std::locale())
	{
		auto pos = sa::ifind_last(dest, search, loc);
		if (pos == dest.end())
			return dest;
		return sa::replace_copy(dest, std::distance(dest.begin(), pos), search.size(), replace);
	}

	/* ERASE LAST */

	template<typename String>
	void erase_last(String& dest, const String& search)
	{
		auto pos = sa::find_last(dest, search);
		if (pos != dest.end())
			sa::erase(dest, std::distance(dest.begin(), pos), search.size());
	}

	template<typename String>
	String erase_last_copy(const String& dest, const String& search)
	{
		auto pos = sa::find_last(dest, search);
		if (pos == dest.end())
			return dest;
		return sa::erase_copy(dest, std::distance(dest.begin(), pos), search.size());
	}

	template<typename String>
	void ierase_last(String& dest, const String& search, const std::locale& loc = std::locale())
	{
		auto pos = sa::ifind_last(dest, search, loc);
		if (pos != dest.end())
			erase(dest, std::distance(dest.begin(), pos), search.size());
	}

	template<typename String>
	String ierase_last_copy(const String& dest, const String& search, const std::locale& loc = std::locale())
	{
		auto pos = sa::ifind_last(dest, search, loc);
		if (pos == dest.end())
			return dest;
		return sa::erase_copy(dest, std::distance(dest.begin(), pos), search.size());
	}

	/* REPLACE ALL */

	template<typename String>
	void replace_all(String& dest, const String& search, const String& replace)
	{
		while (true)
		{
			auto pos = sa::find(dest, search);
			if (pos == dest.end())
				break;
			sa::replace(dest, std::distance(dest.begin(), pos), search.size(), replace);
		}
	}

	template<typename String>
	String replace_all_copy(const String& dest, const String& search, const String& replace)
	{
		String ret(dest);
		sa::replace_all(ret, search, replace);
		return ret;
	}

	template<typename String>
	void ireplace_all(String& dest, const String& search, const String& replace, const std::locale& loc = std::locale())
	{
		while (true)
		{
			auto pos = sa::ifind(dest, search, loc);
			if (pos == dest.end())
				break;
			sa::replace(dest, std::distance(dest.begin(), pos), search.size(), replace);
		}
	}

	template<typename String>
	String ireplace_all_copy(const String& dest, const String& search, const String& replace, const std::locale& loc = std::locale())
	{
		String ret(dest);
		sa::ireplace_all(ret, search, replace, loc);
		return ret;
	}

	/* ERASE ALL */

	template<typename String>
	void erase_all(String& dest, const String& search)
	{
		while (true)
		{
			auto pos = sa::find(dest, search);
			if (pos == dest.end())
				break;
			sa::erase(dest, std::distance(dest.begin(), pos), search.size());
		}
	}

	template<typename String>
	String erase_all_copy(const String& dest, const String& search)
	{
		String ret(dest);
		sa::erase_all(ret, search);
		return ret;
	}

	template<typename String>
	void ierase_all(String& dest, const String& search, const std::locale& loc = std::locale())
	{
		while (true)
		{
			auto pos = sa::ifind(dest, search, loc);
			if (pos == dest.end())
				break;
			sa::erase(dest, std::distance(dest.begin(), pos), search.size());
		}
	}

	template<typename String>
	String ierase_all_copy(const String& dest, const String& search, const std::locale& loc = std::locale())
	{
		String ret(dest);
		sa::ierase_all(ret, search, loc);
		return ret;
	}

} // namespace string_algo

#endif // STRING_ALGO_HPP
