#ifndef _UTF8_UTIL_H
#define _UTF8_UTIL_H
#include <string.h>
#include <vector>
typedef unsigned int WideChar;
typedef unsigned char uint8_t;;

class utf8_util
{
public:
   utf8_util(void);
public:
   ~utf8_util(void);

   WideChar parseUtf8(const char* strPtr);
   void encodeUtf8(WideChar c, char*& strPtr);
   bool isValidUtf8(const char* strPtr);
   bool isUpperCharacter(WideChar c);
   bool isLowerCharacter(WideChar c);
   bool isSpecialCharacter(WideChar c);
   WideChar makeUpperCase(WideChar c);
   WideChar makeLowerCase(WideChar c);


};

#endif _UTF8_UTIL_H
