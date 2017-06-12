#pragma once
#include "Config.hpp"

NAMESPACE_BEGIN

struct __true_type {
};

struct __false_type {
};

#define __TEMPLATE_NULL template<>

template <class Tp>
struct type_traits
{
	typedef __true_type     this_dummy_member_must_be_first;
	typedef __false_type    has_trivial_default_constructor;
	typedef __false_type    has_trivial_copy_constructor;
	typedef __false_type    has_trivial_assignment_operator;
	typedef __false_type    has_trivial_destructor;
	typedef __false_type    is_POD_type;
};

__TEMPLATE_NULL struct type_traits < bool >
{
	typedef __true_type    has_trivial_default_constructor;
	typedef __true_type    has_trivial_copy_constructor;
	typedef __true_type    has_trivial_assignment_operator;
	typedef __true_type    has_trivial_destructor;
	typedef __true_type    is_POD_type;
};

__TEMPLATE_NULL struct type_traits < char >
{
	typedef __true_type    has_trivial_default_constructor;
	typedef __true_type    has_trivial_copy_constructor;
	typedef __true_type    has_trivial_assignment_operator;
	typedef __true_type    has_trivial_destructor;
	typedef __true_type    is_POD_type;
};

__TEMPLATE_NULL struct type_traits < signed char >
{
	typedef __true_type    has_trivial_default_constructor;
	typedef __true_type    has_trivial_copy_constructor;
	typedef __true_type    has_trivial_assignment_operator;
	typedef __true_type    has_trivial_destructor;
	typedef __true_type    is_POD_type;
};

__TEMPLATE_NULL struct type_traits < unsigned char >
{
	typedef __true_type    has_trivial_default_constructor;
	typedef __true_type    has_trivial_copy_constructor;
	typedef __true_type    has_trivial_assignment_operator;
	typedef __true_type    has_trivial_destructor;
	typedef __true_type    is_POD_type;
};

__TEMPLATE_NULL struct type_traits < wchar_t >
{
	typedef __true_type    has_trivial_default_constructor;
	typedef __true_type    has_trivial_copy_constructor;
	typedef __true_type    has_trivial_assignment_operator;
	typedef __true_type    has_trivial_destructor;
	typedef __true_type    is_POD_type;
};

__TEMPLATE_NULL struct type_traits < short > {
	typedef __true_type    has_trivial_default_constructor;
	typedef __true_type    has_trivial_copy_constructor;
	typedef __true_type    has_trivial_assignment_operator;
	typedef __true_type    has_trivial_destructor;
	typedef __true_type    is_POD_type;
};

__TEMPLATE_NULL struct type_traits < unsigned short > {
	typedef __true_type    has_trivial_default_constructor;
	typedef __true_type    has_trivial_copy_constructor;
	typedef __true_type    has_trivial_assignment_operator;
	typedef __true_type    has_trivial_destructor;
	typedef __true_type    is_POD_type;
};

__TEMPLATE_NULL struct type_traits < int > {
	typedef __true_type    has_trivial_default_constructor;
	typedef __true_type    has_trivial_copy_constructor;
	typedef __true_type    has_trivial_assignment_operator;
	typedef __true_type    has_trivial_destructor;
	typedef __true_type    is_POD_type;
};

__TEMPLATE_NULL struct type_traits < unsigned int > {
	typedef __true_type    has_trivial_default_constructor;
	typedef __true_type    has_trivial_copy_constructor;
	typedef __true_type    has_trivial_assignment_operator;
	typedef __true_type    has_trivial_destructor;
	typedef __true_type    is_POD_type;
};

__TEMPLATE_NULL struct type_traits < long > {
	typedef __true_type    has_trivial_default_constructor;
	typedef __true_type    has_trivial_copy_constructor;
	typedef __true_type    has_trivial_assignment_operator;
	typedef __true_type    has_trivial_destructor;
	typedef __true_type    is_POD_type;
};

__TEMPLATE_NULL struct type_traits < unsigned long > {
	typedef __true_type    has_trivial_default_constructor;
	typedef __true_type    has_trivial_copy_constructor;
	typedef __true_type    has_trivial_assignment_operator;
	typedef __true_type    has_trivial_destructor;
	typedef __true_type    is_POD_type;
};

__TEMPLATE_NULL struct type_traits < long long > {
	typedef __true_type    has_trivial_default_constructor;
	typedef __true_type    has_trivial_copy_constructor;
	typedef __true_type    has_trivial_assignment_operator;
	typedef __true_type    has_trivial_destructor;
	typedef __true_type    is_POD_type;
};

__TEMPLATE_NULL struct type_traits < unsigned long long > {
	typedef __true_type    has_trivial_default_constructor;
	typedef __true_type    has_trivial_copy_constructor;
	typedef __true_type    has_trivial_assignment_operator;
	typedef __true_type    has_trivial_destructor;
	typedef __true_type    is_POD_type;
};

#define CLASS_PARTIAL_SPECIALIZATION
#ifdef CLASS_PARTIAL_SPECIALIZATION

template <class _Tp>
struct type_traits < _Tp* > {
	typedef __true_type    has_trivial_default_constructor;
	typedef __true_type    has_trivial_copy_constructor;
	typedef __true_type    has_trivial_assignment_operator;
	typedef __true_type    has_trivial_destructor;
	typedef __true_type    is_POD_type;
};

#else /* CLASS_PARTIAL_SPECIALIZATION */

__TEMPLATE_NULL struct type_traits < char* > {
	typedef __true_type    has_trivial_default_constructor;
	typedef __true_type    has_trivial_copy_constructor;
	typedef __true_type    has_trivial_assignment_operator;
	typedef __true_type    has_trivial_destructor;
	typedef __true_type    is_POD_type;
};

__TEMPLATE_NULL struct type_traits < signed char* > {
	typedef __true_type    has_trivial_default_constructor;
	typedef __true_type    has_trivial_copy_constructor;
	typedef __true_type    has_trivial_assignment_operator;
	typedef __true_type    has_trivial_destructor;
	typedef __true_type    is_POD_type;
};

__TEMPLATE_NULL struct type_traits < unsigned char* > {
	typedef __true_type    has_trivial_default_constructor;
	typedef __true_type    has_trivial_copy_constructor;
	typedef __true_type    has_trivial_assignment_operator;
	typedef __true_type    has_trivial_destructor;
	typedef __true_type    is_POD_type;
};

__TEMPLATE_NULL struct type_traits < const char* > {
	typedef __true_type    has_trivial_default_constructor;
	typedef __true_type    has_trivial_copy_constructor;
	typedef __true_type    has_trivial_assignment_operator;
	typedef __true_type    has_trivial_destructor;
	typedef __true_type    is_POD_type;
};

__TEMPLATE_NULL struct type_traits < const signed char* > {
	typedef __true_type    has_trivial_default_constructor;
	typedef __true_type    has_trivial_copy_constructor;
	typedef __true_type    has_trivial_assignment_operator;
	typedef __true_type    has_trivial_destructor;
	typedef __true_type    is_POD_type;
};

__TEMPLATE_NULL struct type_traits < const unsigned char* > {
	typedef __true_type    has_trivial_default_constructor;
	typedef __true_type    has_trivial_copy_constructor;
	typedef __true_type    has_trivial_assignment_operator;
	typedef __true_type    has_trivial_destructor;
	typedef __true_type    is_POD_type;
};

#endif /* CLASS_PARTIAL_SPECIALIZATION */


// The following could be written in terms of numeric_limits.  
// We're doing it separately to reduce the number of dependencies.

template <class _Tp> struct _Is_integer {
	typedef __false_type _Integral;
};

__TEMPLATE_NULL struct _Is_integer < bool > {
	typedef __true_type _Integral;
};

__TEMPLATE_NULL struct _Is_integer < char > {
	typedef __true_type _Integral;
};

__TEMPLATE_NULL struct _Is_integer < signed char > {
	typedef __true_type _Integral;
};

__TEMPLATE_NULL struct _Is_integer < unsigned char > {
	typedef __true_type _Integral;
};
__TEMPLATE_NULL struct _Is_integer < wchar_t > {
	typedef __true_type _Integral;
};

__TEMPLATE_NULL struct _Is_integer < short > {
	typedef __true_type _Integral;
};

__TEMPLATE_NULL struct _Is_integer < unsigned short > {
	typedef __true_type _Integral;
};

__TEMPLATE_NULL struct _Is_integer < int > {
	typedef __true_type _Integral;
};

__TEMPLATE_NULL struct _Is_integer < unsigned int > {
	typedef __true_type _Integral;
};

__TEMPLATE_NULL struct _Is_integer < long > {
	typedef __true_type _Integral;
};

__TEMPLATE_NULL struct _Is_integer < unsigned long > {
	typedef __true_type _Integral;
};

__TEMPLATE_NULL struct _Is_integer < long long > {
	typedef __true_type _Integral;
};

__TEMPLATE_NULL struct _Is_integer < unsigned long long > {
	typedef __true_type _Integral;
};


NAMESPACE_END