#pragma once
#include "nc_cpp.h"
#include "TypeTraits.h"
#include "Config.hpp"

NAMESPACE_BEGIN

template<class T1, class T2>
inline void _Construct(T1* p, const T2& value)
{
	NcNew((void*)p)T1(value);
}

template <class T1>
inline void _Construct(T1* p)
{
	NcNew((void*)p) T1();
}

template <class Tp>
inline void _Destroy(Tp* pointer)
{
	pointer->~Tp();
}

template <class forward_iterator>
void _destroy_aux(ForwordIterator first, ForwordIterator last, __false_type)
{
	for (; first != last; ++first;)
	{
		destroy(&*first);
	}
}

template <class ForwordIterator>
void _destroy_aux(ForwordIterator first, ForwordIterator last, __true_type) {}

template <class ForwordIterator, Tp>
inline void 
_destroy(ForwordIterator first, ForwordIterator last, Tp*)
{
	typedef typename type_traits<Tp>::has_trivial_destructor trivial_distructor;
	_destroy_aux(first, last, trivial_distructor);
}

template <class ForwordIterator>
inline void _Destroy(ForwordIterator first, ForwordIterator last)
{
	_destroy(first, last, VALUE_TYPE(first));
}

inline void _Destroy(char*, char*) {}
inline void _Destroy(int*, int*) {}
inline void _Destroy(long*, long*) {}
inline void _Destroy(float*, float*) {}
inline void _Destroy(double*, double*) {}
inline void _Destroy(wchar_t*, wchar_t*) {}
inline void _Destroy(cqWCHAR*, cqWCHAR*) {}
inline void _Destroy(cqCHAR*, cqCHAR*) {}

template <class T1, class T2>
inline void construct(T1 *p, const T2& value)
{
	_Construct(p, value);
}

template <class T1>
inline void construct(T1* p)
{
	_Construct(p);
}

template <class Tp>
inline void destroy(Tp * pointer)
{
	_Destroy(pointer);
}

template<class ForwordIterator>
inline void destroy(ForwordIterator first, ForwordIterator last)
{
	_Destroy(first, last);
}

NAMESPACE_END