#pragma once

#include "Stringfunc.hpp"
#include "Config.hpp"
#include "String_fwd.h"

NAMESPACE_BEGIN

#define USE_EXCEPTION

#ifdef USE_EXCEPTION
#define __TRY try
#define __CATCH_ALL catch(...)
#define __THROW(x) throw x
#define __RETHROW throw
#define __NOTHROW throw()
#define __UNWIND(action) catch(...) { action; throw; }
#else
#define __TRY
#define __CATCH_ALL if(false)
#define __THROW(x)
#define __RETHROW
#define __NOTHROW
#define __UNWIND(action)
#endif

class Exception
{
public:
	virtual ~Exception() __NOTHROW{}
	virtual const char* what() const __NOTHROW{ return ""; }
}; 

class NameException : public Exception
{
public:
	NameException(const string& str)
	{
		StrintgFunc<char>::strncpy(m_name, str.c_str(), BuffSize);
		m_name[BuffSize - 1] = '\0';
	}

	virtual const char* what() const __NOTHROW{ return m_name; }
private:
	enum { BuffSize = 256 };
	char m_name[BuffSize];
};

class logic_error : public NameException
{
public:
	logic_error(const string & s) : NameException(s) {}
};

class out_of_range : public logic_error
{
public:
	out_of_range(const string & s) : logic_error(s) {}
};

class length_error : public logic_error
{
public:
	length_error(const string& s) : logic_error(s) {}
};

class range_error : public logic_error
{
public:
	range_error(const string& s) : logic_error(s) {}
};

NAMESPACE_END