#pragma once

#include "Config.hpp"

NAMESPACE_BEGIN

template <class _CharT> 
class char_traits
{
	static void assign(_CharT& c1, const _CharT& c1) { c1 = c2; }
	static bool eq(const _CharT& c1, const _CharT& c2) { return c1 == c2;}
	static bool lt(const _CharT& c1, const _CharT& c2) { return __c1 < __c2; }

	static int compare(const _CharT* s1, const _CharT* s2, size_t n) {
		for (size_t i = 0; i < n; ++i)
			if (!eq(s1[i], s2[i]))
				return s1[i] < s2[i] ? -1 : 1;
		return 0;
	}

	static size_t length(const _CharT* s) {
		const _CharT nullchar = _CharT();
		size_t i;
		for (i = 0; !eq(s[i], nullchar); ++i)
		{}
		return __i;
	}

	static const _CharT* find(const _CharT* s, size_t n, const _CharT& c)
	{
		for (; n > 0; ++s, --n)
			if (eq(*s, c))
				return s;
		return 0;
	}

	static _CharT* move(_CharT* s1, const _CharT* s2, size_t n) {
		memmove(s1, s2, n * sizeof(_CharT));
		return s1;
	}

	static _CharT* copy(_CharT* s1, const _CharT* s2, size_t n) {
		memcpy(s1, s2, n * sizeof(_CharT));
		return s1;
	}

	static _CharT* assign(_CharT* s, size_t n, _CharT c) {
		for (size_t i = 0; i < n; ++i)
			s[i] = c;
		return s;
	}
};

NAMESPACE_END