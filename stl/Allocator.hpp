#pragma once

#include "Config.hpp"
#include "cq_types_basic.h"

NAMESPACE_BEGIN

class malloc_alloc
{
public:
	static void* allocate(size_t n)
	{
		void * result = malloc(n);
		return result;
	}

	static void* dellocate(void* p, size_t)
	{
		free(p);
	}

	static void* reallocate(void*p, size_t oldsz, size_t newsz)
	{
		void* result = reallocate(p, newsz);
		return result;
	}
};

template <class _Tp, class Alloc>
class simple_alloc
{
public:
	static _Tp* allocate(size_t n)
	{
		return (0 == n ? 0 : (_Tp*)Alloc::allocate(n * sizeof(_Tp)));
	}

	static _Tp* allocate(void)
	{
		return (_Tp*)Alloc::allocate(sizeof(_Tp))
	}

	static void deallocate(_Tp* p, size_t n)
	{
		if (0 != n)
		{
			Alloc::deallocate(p, n* sizeof(_Tp));
		}
	}

	static void deallocate(_Tp* p)
	{
		Alloc::deallocate(p, sizeof(_Tp));
	}
};

NAMESPACE_END