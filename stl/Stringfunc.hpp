#pragma once
#include "cq_types_basic.h"
#include "cq_stdlib.h"
#include "Config.hpp"

NAMESPACE_BEGIN

template<class CharT>
class StrintgFunc
{
public:
	static itoc(int32 value);
	static size_t strlen(const CharT* s);
	static CharT*  strcpy(CharT* s1, const CharT* s2);
	static CharT* strcat(CharT* s1, const CharT* s2);
	static int32 strcmp(const CharT* s1, const CharT* s2);
	static int32 strncmp(const CharT* s1, const CharT* s2, size_t count);
	static int32 stricmp(const CharT* s1, const CharT* s2);
	static int32 strnicmp(const CharT* p, const CharT* q, size_t count);
	static int32 atoi(const CharT* s);
	static int32 _atoi(const CharT** s);
	static uint32 atoui_hex(const CharT* str);
	static size_t atozu_hex(const CharT* str);
	static float atof(const CharT* s);
	static double atod(const CharT* s);
	static double _atod(const CharT** s);
	static CharT* itoa(int32 value, CharT* s, int32 radix);
	static CharT* uitoa(uint32 value, CharT* s, int32 radix);
	static CharT* strstr(const CharT* s1, const CharT* s2);
	static CharT* strrstr(const CharT *s1, const CharT *s2);
	static CharT* strupr(CharT* s);
	static CharT* strlwr(CharT* s);
	static int32 sprintf(CharT* buffer, const CharT* format, ...);	///< See remarks of swprintf()
	static CharT* strchr(const CharT* s, CharT c);
	static CharT* strrchr(const CharT* s, CharT c);
	static CharT* strncpy(CharT* s1, const CharT* s2, size_t count);
	static CharT* strncat(CharT* s1, const CharT* s2, size_t count);
	static CharT* strtok_s(CharT* buf, const CharT* spliters, CharT** context);
	static CharT* strtrim(CharT* buf);
	static int strcpy_s(CharT* dest, size_t destSize, const CharT* src);
	static int strcat_s(CharT* dest, size_t destSize, const CharT* src);
	BOOL strStartsWith(const CharT* str, const CharT* start);
	BOOL strEndsWith(const CharT* str, const CharT* end);
#define strReplace(str, from, to) {char* p = (str); for( ; *p; p++) { if (*p == (from)) *p = (to); }}
#define wcsReplace(str, from, to) {cqWCHAR* p = (str); for( ; *p; p++) { if (*p == (from)) *p = (to); }}
};	

template<>
class StrintgFunc<char>
{
public:
	static char itoc(int32 value) {  return itoc(value);  }
	static size_t strlen(const char* s) { return strlen(s); }
	static char*  strcpy(char* s1, const char* s2) { return strcpy(s1, s2); }
	static char* strcat(char* s1, const char* s2) { return strcat(s1, s2); }
	static int32 strcmp(const char* s1, const char* s2) { return strcmp(s1, s2); }
	static int32 strncmp(const char* s1, const char* s2, size_t count) { return strncmp(s1, s2, count); }
	static int32 stricmp(const char* s1, const char* s2) { return stricmp(s1, s2); }
	static int32 strnicmp(const char* p, const char* q, size_t count) { return strnicmp(p, q, count); }
	static int32 atoi(const char* s) { return atoi(s); }
	static int32 _atoi(const char** s) { return _atoi(s); }
	static uint32 atoui_hex(const char* str) { return atoui_hex(str); }
	static size_t atozu_hex(const char* str) { return atozu_hex(str); }
	static float atof(const char* s) { atof(s); }
	static double atod(const char* s) { atod(s); }
	static double _atod(const char** s) { _atod(s); }
	static char* itoa(int32 value, char* s, int32 radix) { return itoa(value, s, radix); }
	static char* uitoa(uint32 value, char* s, int32 radix) { return uitoa(value, s, radix); }
	static char* strstr(const char* s1, const char* s2) { return strstr(s1, s2); }
	static char* strrstr(const char *s1, const char *s2) { return strrstr(s1, s2); }
	static char* strupr(char* s) { return strupr(s); }
	static char* strlwr(char* s) { return strlwr(s); }
	//static int32 sprintf(char* buffer, const char* format, ...); { return sprintf(buffer, format); }	///< See remarks of swprintf()
	static char* strchr(const char* s, char c) { return strchr(s, c); }
	static char* strrchr(const char* s, char c) { return strrchr(s, c); }
	static char* strncpy(char* s1, const char* s2, size_t count)  { return strncpy(s1, s2, count); }
	static char* strncat(char* s1, const char* s2, size_t count)  { return strncat(s1, s2, count); }
	static char* strtok_s(char* buf, const char* spliters, char** context) { return strtok_s(buf, spliters, context); }
	static char* strtrim(char* buf) { return strtrim(buf); }
	static int strcpy_s(char* dest, size_t destSize, const char* src) { return strcpy_s(dest, destSize, src); }
	static int strcat_s(char* dest, size_t destSize, const char* src) { return strcat_s(dest, destSize, src); }
	BOOL strStartsWith(const char* str, const char* start) { return strStartsWith(str, start); }
	BOOL strEndsWith(const char* str, const char* end) { return strEndsWith(str, end); }
#define strReplace(str, from, to) {char* p = (str); for( ; *p; p++) { if (*p == (from)) *p = (to); }}
#define wcsReplace(str, from, to) {cqWCHAR* p = (str); for( ; *p; p++) { if (*p == (from)) *p = (to); }}
};


NAMESPACE_END

