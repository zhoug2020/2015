#ifndef _FILE_CHECK_H
#define _FILE_CHECK_H
#include <string.h>
#include <vector>

const int LINEBUFF_SIZE = 1024;
const std::string TAB_REPLACE = "   ";
const std::string TAB_STRING = "\t";
const std::string WINDOWS_RETURN = "\r\n";
const std::string UNIX_RETURN = "\n";

class FileCheck
{
public:
	FileCheck(void);
	~FileCheck(void);

   bool check(const std::vector<std::string>& inFiles);

private:
   bool checkOneFile(const std::string& inFileName);
   void replaceString(const std::string & seach, const std::string& replace,std::string & inoutStr);
   void trim(std::string &line);
   std::string getTempFileName(const std::string fileName);
   void updateFile(const std::string& inFile, const std::string& outFile);
};

#endif _FILE_CHECK_H