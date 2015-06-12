#include "StdAfx.h" // delete later
#include "utf8_util.h"

/** UTF 8 format
* Unicode character scope          |   UTF-8 encoding mode
* (sixteen )                      | £¨bianry£©
* --------------------+---------------------------------------------
* 0000 0000-0000 007F   | 0xxxxxxx
* 0000 0080-0000 07FF   | 110xxxxx 10xxxxxx
* 0000 0800-0000 FFFF  | 1110xxxx 10xxxxxx 10xxxxxx
* 0001 0000-0010 FFFF  | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
*/

utf8_util::utf8_util(void)
{
}

utf8_util::~utf8_util(void)
{
}


WideChar parseUtf8(const char* strPtr)
{
   if (!strPtr)
   {
      return 0;
   }

   WideChar c = static_cast<uint8_t>(*strPtr++);

   if ((c & 0x80) == 0xc0)
   {
      uint8_t b2 = static_cast<uint8_t>(*strPtr++);
      c = ((c & 0x1f) << 6) | (b2 & 0x3f);
   }
   else if ((c & 0xf0) == 0xe0)
   {
      uint8_t b2 = static_cast<uint8_t>(*strPtr++);
      uint8_t b3 = static_cast<uint8_t>(*strPtr++);
      c = ((c & 0xf) << 12) | ((b2 & 0x3f) << 6) | (b3 & 0x3f);
   }
   else 
   {
      uint8_t b2 = static_cast<uint8_t>(*strPtr++);
      uint8_t b3 = static_cast<uint8_t>(*strPtr++);
      uint8_t b4 = static_cast<uint8_t>(*strPtr++);
      c = ((c & 0x7) << 18) | ((b2 & 0x3f) << 12) | ((b3 & 0x3f) << 6) | (b4 & 0x3f);
   }
   return c;
}

void encodeUtf8(WideChar c, char*& strPtr)
{
   if (c < 0x80)
   {
      *strPtr++ = c;
   }
   else if (c < 0x800)
   {
      *strPtr++ = (c>>6) | 0xc0;
      *strPtr++ = (c & 0x3f) | 0x80; 
   }
   else if (c < 10000)
   {
      *strPtr++ = (c>>12) | 0xe0;
      *strPtr++ = ((c>>6) & 0x3f) | 0x80;
      *strPtr++ = (c & 0x3f) | 0x80; 
   }
   else if (c < 0x10ffff)
   {
      *strPtr++ = (c>>18) | 0xf0;
      *strPtr++ = ((c>>12) & 0x3f) | 0x80;
      *strPtr++ = ((c>>6) & 0x3f) | 0x80;
      *strPtr++ = (c & 0x3f) | 0x80; 
   }
}

bool isValidUtf8(const char* strPtr)
{
 /*  if (!strPtr)
   {
      return false;
   }

   while(strPtr)
   {
      uint8_t c = static_cast<uint8_t>(*strPtr++);
      if (c & 0x80)
      {
         if ((c & 0x80) == 0xc0)
         {
            uint8_t b2 = static_cast<uint8_t>(*strPtr++);
            if ((b2 & 0x80) != 0x80)
            {
               return false;
            }

         }
      }

          
   }*/
   return true;

}

bool isUpperCharacter(WideChar c)
{
   if (c < 0x80) //ASCII
   {
      return (c >= 'A' && c <= 'Z');
   }
   else if ( c < 0x100) //ISO 8859-1
   {
      return (c >= 0xc0 && c<=0xde);
   }

   return false;
}

bool isLowerCharacter(WideChar c)
{
   if (c < 0x80) //ASCII
   {
      return (c >= 'a' && c <= 'z');
   }
   else if ( c < 0x100) //ISO 8859-1
   {
      return (c >= 0xe0);
   }

   return false;
}

bool isSpecialCharacter(WideChar c)
{
   if ( isspace(c) || iscntrl(c) || ispunct(c))
   {
      return true;
   }
   return false;
}

WideChar makeUpperCase(WideChar c)
{
   if (c < 0x80) //ASCII
   {
      if (c >= 'a' && c <= 'z')
      {
         return c + ('A' - 'a');
      }
   }
   else if ( c < 0x100) //ISO 8859-1
   {
      if ((c >= 0xe0 && c <= 0xfe))
      {
         return c - 0x20;
      }
   }
   return c;
}

WideChar makeLowerCase(WideChar c)
{
   if (c < 0x80) //ASCII
   {
      if (c >= 'A' && c <= 'Z')
      {
         return ( c + 'a' - 'A');
      }
   }
   else if ( c < 0x100) //ISO 8859-1
   {
      if ((c >= 0xc0 && c <= 0xde))
      {
         return c+0x20;
      }
   }
   return c;
}
