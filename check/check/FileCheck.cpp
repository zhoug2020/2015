#include "stdafx.h" //remove later
#include "FileCheck.h"
#include <time.h>

FileCheck::FileCheck(void)
{
}

FileCheck::~FileCheck(void)
{
}

bool FileCheck::check(const std::vector<std::string>& inFiles)
{
   std::vector<std::string>::const_iterator it = inFiles.begin();
   for(; it != inFiles.end(); ++it)
   {
      checkOneFile(*it);
   }
   return true;
}

bool FileCheck::checkOneFile(const std::string& inFileName)
{
   if (inFileName.empty())
	{
		return false;
	}

	FILE * fp = NULL;
   FILE * fpCopy = NULL;
	
   fp = fopen(inFileName.c_str(), "rb");

   std::string temFileName = getTempFileName(inFileName);

   fpCopy = fopen(temFileName.c_str(),"wb");

   if (!fp || !fpCopy)
   {
      return false;
   }

   while (!feof(fp))
	{
      char buffer[LINEBUFF_SIZE + 1];
      memset(buffer,0,sizeof(buffer));
      int linNum = 1;
      if (fgets(buffer,LINEBUFF_SIZE,fp))
      {
         std::string line = buffer;
         trim(line);
         replaceString(TAB_STRING,TAB_REPLACE,line);
         replaceString(WINDOWS_RETURN,UNIX_RETURN,line);
         memcpy(buffer,line.c_str(),line.size());
         fwrite(buffer,sizeof(char),line.size(),fpCopy);
         linNum++;
      }
	}

   if (fp)
   {
      fclose(fp);
   }

   if(fpCopy)
   {
      fclose(fpCopy);
   }

   updateFile(temFileName,inFileName);

   remove(temFileName.c_str());

   return true;
}

void FileCheck::replaceString(const std::string & seach, const std::string& replace,std::string & inoutStr)
{
   if (!inoutStr.empty())
   {
      size_t pos = 0;
      pos = inoutStr.find(seach,pos);
      while (pos != std::string::npos)
      {
         inoutStr.replace(pos,seach.size(),replace);
         pos+=seach.size();
         pos = inoutStr.find(seach,pos);
      }
   }
}

void FileCheck::trim(std::string &line)
{
   UINT32 length = 0;
   if (!line.empty())
   {
      size_t endPos = line.length() - 1;
      while (endPos)
      {
         if (isspace(line[endPos]))
         {
            endPos --;
         }
         else
         {
            break;
         }
      }

      if (endPos < line.length() - 1 )
      {
         line.replace(endPos+1,(line.length() - endPos - 2),"");
      }
   }

}

std::string FileCheck::getTempFileName(const std::string fileName)
{
   char buffer[255];
   memset(buffer,0,255);
   const time_t t = time(NULL);
   struct tm* current_time = localtime(&t);
   if (current_time)
   {
      sprintf_s(buffer,"%d%d%d%d%d%d",
         current_time->tm_year + 1900,
         current_time->tm_mon + 1,
         current_time->tm_mday,
         current_time->tm_hour,
         current_time->tm_min,
         current_time->tm_sec);
   }

   std::string tmpFileName;
   size_t dotPos = fileName.rfind('.');
   if (dotPos != std::string::npos)
   {
      std::string extName = fileName.substr(dotPos);
      tmpFileName = fileName.substr(0,dotPos);
      tmpFileName += "_template";
      tmpFileName += buffer;
      tmpFileName += extName;
   }
   return tmpFileName;
}

void FileCheck::updateFile(const std::string& inFile, const std::string& outFile)
{
   if (inFile.empty() || outFile.empty())
   {
      return;
   }

   FILE * fpFrom = fopen(inFile.c_str(),"rb");
   FILE * fpTo = fopen(outFile.c_str(),"wb");
   
   if (fpFrom && fpTo)
   {
      fseek(fpFrom,0,SEEK_END);
      long len = ftell(fpFrom);
      fseek(fpFrom,0,SEEK_SET);
      char *temp = new char[len];
      memset(temp,0,len);
      fread(temp,sizeof(char),len,fpFrom);
      fwrite(temp,sizeof(char),len,fpTo);
      delete [] temp;
      fclose(fpFrom);
      fclose(fpTo);
   }
}