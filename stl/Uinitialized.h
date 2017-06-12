#pragma once
#include "Config.hpp"

NAMESPACE_BEGIN
#include <string.h>

template <class InputIter, class ForwardIter>
inline ForwardIter 
__uninitialized_copy_aux(InputIter first, InputIter last, ForwardIter result, __true_type)
{
	return copy(first, last, result);
}

template <class InputIter, class ForwardIter>
inline ForwardIter
__uninitialized_copy_aux(InputIter first, InputIter last, ForwardIter result, __false_type)
{
	ForwardIter cur = result;
	__TRY{
		for (; first != last; ++first, ++cur)
		{
			_Construct(&*cur, *first);
		}
		return cur;
	}
	__UNWIND(_Destroy(result, cur))
}

template <class InputIter, class ForwardIter, class Tp>
inline ForwardIter
__uninitialized_copy(InputIter first, InputIter last, ForwardIter result, Tp*)
{
	typedef typename type_traits<Tp>::is_POD_type IS_POD;
	return __uninitialized_copy_aux(first, last, result, IS_POD());
}

template <class InputIter, class ForwardIter>
inline ForwardIter
uninitialized_copy(InputIter first, InputIter last, ForwardIter result)
{
	return __uninitialized_copy(first, last, VALUE_TYPE(result));
}

inline char* uninitialized_copy(const char* first, const char* last, char* result) 
{
	memmove(result, first, last - first);
	return result + (last - first);
}


inline wchar_t* uninitialized_copy(const wchar_t* first, const wchar_t* last, wchar_t* result)
{
	memmove(result, first, sizeof(wchar_t) * (last - first));
	return result + (last - first);
}

// Valid if copy construction is equivalent to assignment, and if the
// destructor is trivial.
template <class ForwardIter, class Tp>
inline void  _uninitialized_fill_aux(ForwardIter first, ForwardIter last, const Tp& x, __true_type)
{
	fill(first, last, x);
}

template <class ForwardIter, class Tp>
void _uninitialized_fill_aux(ForwardIter first, ForwardIter last, const Tp& x, __false_type)
{
	ForwardIter cur = first;
	__TRY{
		for (; cur != last; ++cur)
		_Construct(&*cur, x);
	}
	__UNWIND(_Destroy(first, cur));
}

template <class ForwardIter, class Tp, class Tp1>
inline void _uninitialized_fill(ForwardIter first, ForwardIter last, const Tp& x, Tp1*)
{
	typedef typename type_traits<Tp1>::is_POD_type _Is_POD;
	_uninitialized_fill_aux(first, last, x, _Is_POD());

}

template <class ForwardIter, class Tp>
inline void uninitialized_fill(ForwardIter first, ForwardIter last, const Tp& x)
{
	_uninitialized_fill(first, last, x, VALUE_TYPE(__first));
}

// Valid if copy construction is equivalent to assignment, and if the
//  destructor is trivial.
template <class _ForwardIter, class _Size, class _Tp>
inline _ForwardIter
_uninitialized_fill_n_aux(_ForwardIter __first, _Size __n, const _Tp& __x, __true_type)
{
	return fill_n(__first, __n, __x);
}

template <class _ForwardIter, class _Size, class _Tp>
_ForwardIter
 _uninitialized_fill_n_aux(_ForwardIter __first, _Size __n, const _Tp& __x, __false_type)
{
	_ForwardIter __cur = __first;
	__TRY{
		for (; __n > 0; --__n, ++__cur)
		_Construct(&*__cur, __x);
		return __cur;
	}
	__UNWIND(_Destroy(__first, __cur));
}

template <class _ForwardIter, class _Size, class _Tp, class _Tp1>
inline _ForwardIter
_uninitialized_fill_n(_ForwardIter __first, _Size __n, const _Tp& __x, _Tp1*)
{
	typedef typename type_traits<_Tp1>::is_POD_type _Is_POD;
	return _uninitialized_fill_n_aux(__first, __n, __x, _Is_POD());
}

template <class _ForwardIter, class _Size, class _Tp>
inline _ForwardIter
uninitialized_fill_n(_ForwardIter __first, _Size __n, const _Tp& __x)
{
	return _uninitialized_fill_n(__first, __n, __x, VALUE_TYPE(__first));
}

// Extensions: __uninitialized_copy_copy, __uninitialized_copy_fill, 
// __uninitialized_fill_copy.

// __uninitialized_copy_copy
// Copies [first1, last1) into [result, result + (last1 - first1)), and
//  copies [first2, last2) into
//  [result, result + (last1 - first1) + (last2 - first2)).

template <class _InputIter1, class _InputIter2, class _ForwardIter>
inline _ForwardIter
_uninitialized_copy_copy(_InputIter1 __first1, _InputIter1 __last1,
_InputIter2 __first2, _InputIter2 __last2, _ForwardIter __result)
{
	_ForwardIter __mid = uninitialized_copy(__first1, __last1, __result);
	__TRY{
		return uninitialized_copy(__first2, __last2, __mid);
	}
	__UNWIND(_Destroy(__result, __mid));
}

// __uninitialized_fill_copy
// Fills [result, mid) with x, and copies [first, last) into
//  [mid, mid + (last - first)).
template <class _ForwardIter, class _Tp, class _InputIter>
inline _ForwardIter
_uninitialized_fill_copy(_ForwardIter __result, _ForwardIter __mid, const _Tp& __x,
_InputIter __first, _InputIter __last)
{
	uninitialized_fill(__result, __mid, __x);
	__TRY{
		return uninitialized_copy(__first, __last, __mid);
	}
	__UNWIND(_Destroy(__result, __mid));
}

// __uninitialized_copy_fill
// Copies [first1, last1) into [first2, first2 + (last1 - first1)), and
//  fills [first2 + (last1 - first1), last2) with x.
template <class _InputIter, class _ForwardIter, class _Tp>
inline void
_uninitialized_copy_fill(_InputIter __first1, _InputIter __last1,
_ForwardIter __first2, _ForwardIter __last2,
const _Tp& __x)
{
	_ForwardIter __mid2 = uninitialized_copy(__first1, __last1, __first2);
	__TRY{
		uninitialized_fill(__mid2, __last2, __x);
	}
	__UNWIND(_Destroy(__first2, __mid2));
}

NAMESPACE_END