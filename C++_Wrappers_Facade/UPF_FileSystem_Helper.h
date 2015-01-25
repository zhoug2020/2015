#ifndef UPF_FILESYSTEM_HELPER_H_
#define UPF_FILESYSTEM_HELPER_H_

#include <algorithm>
#include <string>
#include "..\OS_Adapter\UPF_OS.h"

namespace UPF{
namespace FileSystem{

template <unsigned long ATTR>
struct file_attr_filter 
{
	static bool is_attr(unsigned long file_attr){	return (file_attr & ATTR) == ATTR;	}
};
class UPF_Export file_find_helper
{
public:
	typedef std::pair<bool,UPF_HANDLE> FD_TYPE;
	static	bool is_direcrory(UPF_find_data_t & find_data)
	{	
		return file_attr_filter<UPF_File_Flags::UPF_FILE_ATTRIBUTE_DIRECTORY>::is_attr(find_data.dwFileAttributes);
	}

	static	bool is_hidden(UPF_find_data_t & find_data)
	{
		return file_attr_filter<UPF_File_Flags::UPF_FILE_ATTRIBUTE_HIDDEN>::is_attr(find_data.dwFileAttributes);
	}
	static	bool is_read_only(UPF_find_data_t & find_data)
	{
		return file_attr_filter<UPF_File_Flags::UPF_FILE_ATTRIBUTE_READONLY>::is_attr(find_data.dwFileAttributes);
	}
	static	bool is_docs(UPF_find_data_t & find_data);
	static	FD_TYPE first(const char * filespec,UPF_find_data_t & find_data);
	static	FD_TYPE next(UPF_HANDLE find_handle, UPF_find_data_t & find_data);
	static  bool	close(UPF_HANDLE find_handle);	
};
 template<typename T>
 bool enumerate_current_dir(const char * dir_path,T callback_func)
 {
	 bool				ret				= false;
	 UPF_find_data_t	find_data		= {0};
	 UPF_HANDLE			find_handle		= UPF_INVALID_HANDLE ;
	 std::string		find_name_str	= std::string(dir_path) + std::string("\\*");

	 file_find_helper::FD_TYPE		ret_val(false,UPF_INVALID_HANDLE);

	 for(ret_val = file_find_helper::first(find_name_str.c_str(),find_data),ret = ret_val.first, find_handle = ret_val.second;
		 ret!= false;
		 ret = file_find_helper::next(find_handle,find_data).first)
	 {
		 if(file_find_helper::is_docs(find_data))
		 { continue;}	
		 callback_func(find_data);
	 }
	 return true;
 }
 extern UPF_Export  bool delete_files(const char * dir_path);
 extern UPF_Export  bool delete_dir(const char * dir_path);
 extern UPF_Export  bool copy_dir(const char * dest_dir,const char * src_dir);

}
}
#endif