#pragma  once
#include "Config.hpp"

NAMESPACE_BEGIN

#include <string.h>
//#define FUNCTION_TMPL_PARTIAL_ORDER
#define CLASS_PARTIAL_SPECIALIZATION

//--------------------------------------------------
// copy

// All of these auxiliary functions serve two purposes.  (1) Replace
// calls to copy with memmove whenever possible.  (Memmove, not memcpy,
// because the input and output ranges are permitted to overlap.)
// (2) If we're using random access iterators, then write the loop as
// a for loop with an explicit count.

template <class InputIter, class OutputIter ,class Distance>
inline OutputIter _copy(input_iterator first, input_iterator last, OutputIter result)
{
	for (; first != last; ++first)
	{
		*result = *first;
	}
	return result;
}

template <class RandomAccessIter, class OuputIter, class Distance>
inline OutputIter
_copy(RandomAccessIter first, RandomAccessIter last, OutputIter result,
random_access_iterator_tag, Distance*)
{
	for (Distance n = last - first; n > 0; --n)
	{
		*result = *first;
		++first;
		++result;
	}
}

template <class Tp>
_copy_trivial(const Tp* first, Tp* last, Tp* result)
{
	memmove(result, first, sizeof(Tp)*(last - first));
	return result + (last - first);
}

#if defined(FUNCTION_TMPL_PARTIAL_ORDER)
template <class InputIter, class OutputIter>
inline OutputIter _copy_aux2(InputIter first, InputIter last,
	OutputIter result, __false_type)
{
	_copy(first, last, result, 
		ITERATOR_CATEGORY(first),
		DISTANCE_TYPE(first));
}

template <class Tp>
inline Tp* _copy_aux2(Tp* first, Tp* last, Tp* result, __true_type)
{
	return _copy_trivial(first, last, result);
}

template <class Tp>
inline Tp* _copy_aux2(const Tp* first, const Tp* last, Tp* result, __true_type)
{
	return _copy_trivial(first, last, result);
}

template <class InputIter, class OuputIter, class Tp>
inline OutputIter _copy_aux(InputIter first, InputIter last,
	OutputIter result, Tp*)
{
	typedef typename type_traits<Tp>::has_trivial_assignment_operator _Trivial;
	return _copy_aux2(first, last, result, _Trivial());
}

template<class InputIter, class OutputIter>
inline OutputIter copy(InputIter first, InputIter last, OutputIter result)
{
	return _copy_aux(first, last, result, VALUE_TYPE(first));
}

#elif defined (CLASS_PARTIAL_SPECIALIZATION)

// Hack for compilers that don't have partial ordering of function templates
// but do have partial specialization of class templates.
template <class InputIter, class OutputIter, class BoolType>
struct _copy_dispatch
{
	static OutputIter copy(InputIter first, InputIter last, OutputIter result)
	{
		typedef typename iterator_traits<InputIter>::iterator_category Category;
		typedef typename iterator_traits<InputIter>::difference_type Distance;
		return _copy(first, last, result, Category(), (Distance*)0);
	}
};

template <class Tp>
struct _copy_dispatch<Tp*,Tp*,Tp*, __true_type>
{
	static Tp* copy(Tp* first, Tp* last, Tp* result)
	{
		return _copy_trivial(first, last, result);
	}
};

template <class Tp>
struct _copy_dispatch<const Tp*, const Tp*, Tp*, __true_type>
{
	static Tp* copy(const Tp* first, const Tp* last, Tp* result)
	{
		return _copy_trivial(first, last, result);
	}
};

template <class InputIter, class OutputIter>
inline OuputIter copy(InputIter first, InputIter last, OutputIter result)
{
	typedef typename iterator_traits<InputIter>::value_type _Tp;
	typedef typename type_traits<Tp>::has_trivial_assignment_operator _Trivial;
	return _copy_dispatch<InputIter, OutputIter, _Trivial>::copy(first, last, result);
}
#else
// Fallback for compilers with neither partial ordering nor partial
// specialization.  Define the faster version for the basic builtin
// types.

template <class _InputIter, class _OutputIter>
inline _OutputIter copy(_InputIter __first, _InputIter __last,
	_OutputIter __result)
{
	return _copy(__first, __last, __result,
		ITERATOR_CATEGORY(__first),
		DISTANCE_TYPE(__first));
}

#define __DECLARE_COPY_TRIVIAL(_Tp)                                \
  inline _Tp* copy(const _Tp* __first, const _Tp* __last, _Tp* __result) { \
    memmove(__result, __first, sizeof(_Tp) * (__last - __first));          \
    return __result + (__last - __first);                                  \
  }

__DECLARE_COPY_TRIVIAL(char)
__DECLARE_COPY_TRIVIAL(signed char)
__DECLARE_COPY_TRIVIAL(unsigned char)
__DECLARE_COPY_TRIVIAL(short)
__DECLARE_COPY_TRIVIAL(unsigned short)
__DECLARE_COPY_TRIVIAL(int)
__DECLARE_COPY_TRIVIAL(unsigned int)
__DECLARE_COPY_TRIVIAL(long)
__DECLARE_COPY_TRIVIAL(unsigned long)
__DECLARE_COPY_TRIVIAL(wchar_t)
__DECLARE_COPY_TRIVIAL(long long)
__DECLARE_COPY_TRIVIAL(unsigned long long)
__DECLARE_COPY_TRIVIAL(float)
__DECLARE_COPY_TRIVIAL(double)
__DECLARE_COPY_TRIVIAL(long double)
#undef __DECLARE_COPY_TRIVIAL

#endif

//--------------------------------------------------
// copy_backward
#if defined (CLASS_PARTIAL_SPECIALIZATION)
template <class BidirectionInputer1, class BidirectionInputer2, class Distance>
inline BidirectionInputer2 _copy_backword(BidirectionInputer1 first,
	BidirectionInputer1 last,
	BidirectionInputer2 result,
	bidirectional_iterator_tag,
	Distance*)
{
	while (first != last)
	{
		*--result = *--last;
		return result;
	}
}

template <class RandomaccessIter, class BidrectionInputer, class Direction>
inline BidrectionInputer _copy_backword(RandomaccessIter first,
	RandomaccessIter last, BidrectionInputer result, random_access_iterator_tag, Distance*)
{
	for (Direction n = last - first; n > 0; --n)
	{
		*--result = --last;
		return result;
	}
}

template <class BidrectionInputer1, class BidrectionInputer2, class BoolType>
struct _copy_backward_dispatch
{
	typedef typename iterator_traits<BidrectionInputer1>::iterator_category _Cat;
	typedef typename iterator_traits<BidrectionInputer1>::difference_type _Distance;
	static BidrectionInputer2 copy(BidrectionInputer1 first, BidrectionInputer1 last, BidrectionInputer2 result)
	{
		return _copy_backword(first, last, _Cat(), (Distance*)0);
	}
};

template <class Tp>
struct _copy_backward_dispatch<Tp*, Tp*, __true_type>
{
	static Tp* copy(const Tp* first, const Tp* last, Tp* result)
	{
		const ptrdiff_t n = last - first;
		memmove(result - n, first, sizeof(Tp)* n);
		return result - n;
	}
};

template <class Tp>
struct _copy_backward_dispatch <const Tp*, const Tp*, __true_type >
{
	static Tp* copy(const Tp* first, const Tp* last, Tp* result)
	{
		return _copy_backward_dispatch<Tp*, Tp*, __true_type>::copy(first, last, result);
	}
};

template <class B1, class B2>
inline B2 copy_backward(B1 first, B1 last, B2 result)
{
	typedef type_traits<iterator_traits<B2>::value_type>::has_trivial_assignment_operator _Trivial;
	return _copy_backward_dispatch<B1, B2, _Trivial>::copy(first, last, result);
}
#else

template <class _BI1, class _BI2>
inline _BI2 copy_backward(_BI1 __first, _BI1 __last, _BI2 __result) {
	return _copy_backward_dispatch(__first, __last, __result,
		ITERATOR_CATEGORY(__first),
		DISTANCE_TYPE(__first));
}
#endif


//--------------------------------------------------
// fill 
template <class ForwardInputer, class Tp>
void fill(ForwardInputer first, ForwardInputer last, const Tp& value)
{
	for (; first != last, ++first)
	{
		*first = value;
	}
}

template <class OutputIter, class Size, class Tp>
OutputIter fill_n(OutputIter first, Size n, const Tp& value)
{
	for (; n > 0; --n; ++first)
	{
		*first = value;
	}
	return first;
}

// Specialization: for one-byte types we can use memset.
inline void fill(unsigned char* first, unsigned char* last, const unsigned char& c)
{
	unsigned char temp = c;
	memset(first, temp, last - first);
}

inline void fill(signed char* first, signed char* last, const signed char& c)
{
	signed signed char temp = c;
	memset(first, static_cast<unsigned char>(temp), last - first);
}

inline void fill(char* first, char* last, const char& c)
{
	char temp = c;
	memset(first, static_cast<unsigned char>(temp), last - first);
}

template <class Size>
inline unsigned char* fill_n(unsigned char* first, Size n,
	const unsigned char& c) {
	fill(first, first + n, c);
	return first + n;
}

template <class Size>
inline signed char* fill_n(char* first, Size n,
	const signed char& c) {
	fill(first, first + n, c);
	return first + n;
}

template <class Size>
inline char* fill_n(char* first, Size n, const char& c) {
	fill(first, first + n, c);
	return first + n;
}

template<class T>
inline void swap(T& a, T&b)
{
	T temp = a;
	a = b;
	b = temp;
}

template<class InputIter1, class InputIter2>
inline bool equal(InputIter1 first1, InputIter1 last1, InputIter2 first2, InputIter2 last2)
{
	for (; first1 != last1; ++first1, ++first2)
	{
		if (*first1 != *first2)
			return false;
	}

	return true;
}

template<class InputIter1, class InputIter2, class BinaryPridicate>
inline bool equal(InputIter1 first1, InputIter1 last1, InputIter2 first2, InputIter2 last2, BinaryPridicate binary_pred)
{
	for (; first1 != first2; ++first1, ++first2)
		if (!binary_pred(*first1, *first2))
			return false;

	return true;
}


template<class InputIter1, class InputIter2>
bool lexicographical_compare(InputIter1 first1, InputIter1 last1, InputIter2 first2, InputIter2 last2)
{
	for (; first1 != last1 && first2 != last2; ++first1, ++first2)
	{
		if (*first1 < first2)
			return true;
		if (*first2 < *first1)
			return false;
	}
	return first1 == last1 && first2 == last2;
}

NAMESPACE_END