#include "UPF_FileSystem_Helper.h"

#include <stack>
#include <string>
#define BACKSLASH '\\'
#define SLASH	  '/'	

namespace UPF{
namespace FileSystem{

using namespace std;
bool 
file_find_helper::is_docs(UPF_find_data_t & find_data)
{
	bool ret = false;
	if (find_data.cFileName[0] == '.')
	{
		if (find_data.cFileName[1] == '\0' ||
			(find_data.cFileName[1] == '.' &&
			find_data.cFileName[2] == '\0'))
		{
			ret = TRUE;
		}
	}

	return ret;
}

file_find_helper::FD_TYPE 
file_find_helper::first(const char * filespec,UPF_find_data_t & find_data)
{
	FD_TYPE		ret_val(false,UPF_INVALID_HANDLE);
	UPF_HANDLE  find_handle = UPF_INVALID_HANDLE ;

	find_handle = UPF_OS::find_first_file(filespec,&find_data);

	ret_val.first	= (find_handle!=UPF_INVALID_HANDLE)?true:false;
	ret_val.second	= find_handle;

	return ret_val;
}

file_find_helper::FD_TYPE 
file_find_helper::next(UPF_HANDLE find_handle, UPF_find_data_t & find_data)
{	
	FD_TYPE	ret_val(false,UPF_INVALID_HANDLE);

	ret_val.first	= (UPF_OS::find_next_file(find_handle,&find_data)==0)?true:false;
	ret_val.second  = find_handle;

	return ret_val;
}
bool	
file_find_helper::close(UPF_HANDLE find_handle)
{
	return UPF_OS::find_close(find_handle)==0?true:false;
}


string &   
append_slash_at_path_end(string & path)
{
	if(path.size()>0) 
	{
		char last_char = path[path.size()-1];
		if( last_char != BACKSLASH && last_char != SLASH )
		{
			path+=SLASH;
		}
	}
	return path;
}

bool 
delete_files(const char * dir_path)
{
	string			find_name_str	= dir_path;
	UPF_find_data_t	find_data		= {0};
	UPF_HANDLE		find_handle		= UPF_INVALID_HANDLE ;
	stack<string>	find_dirs;	
	bool			ret				= false;
	file_find_helper::FD_TYPE		ret_val(false,UPF_INVALID_HANDLE);

	find_dirs.push(dir_path); 
	while(!find_dirs.empty())
	{
		memset(&find_data,0,sizeof(find_data));
		string path  = find_dirs.top();
		find_name_str = append_slash_at_path_end(path) + "*";
		find_dirs.pop();

		for(ret_val = file_find_helper::first(find_name_str.c_str(),find_data),ret = ret_val.first, find_handle = ret_val.second;
			ret!= false;
			ret = file_find_helper::next(find_handle,find_data).first)
		{
			if(file_find_helper::is_docs(find_data))
			{ continue;}		
			
			if(file_find_helper::is_direcrory(find_data))
			{	
				find_dirs.push(append_slash_at_path_end(path) + UPF_Wide_To_Ascii(find_data.cFileName).char_rep());
				continue;
			}
			else
			{
				if(UPF_OS::delete_file( (append_slash_at_path_end(path) + UPF_Wide_To_Ascii(find_data.cFileName).char_rep()).c_str())!=0)
				{
					file_find_helper::close(find_handle);
					return false;
				}
			}			
		}// for
			file_find_helper::close(find_handle);
	}// while

	return true;
}

bool 
delete_dir(const char * dir_path)
{
	string			find_name_str	= dir_path;
	UPF_find_data_t	find_data		= {0};
	UPF_HANDLE		find_handle		= UPF_INVALID_HANDLE ;
	stack<string>	find_dirs;
	stack<string>	delete_dirs;
	bool			ret				= false;
	file_find_helper::FD_TYPE		ret_val(false,UPF_INVALID_HANDLE);

	if(!delete_files(dir_path))
		return false;

	find_dirs.push(dir_path); 
	delete_dirs.push(dir_path); 

	while(!find_dirs.empty())
	{
		memset(&find_data,0,sizeof(find_data));
		string path  = find_dirs.top();
		delete_dirs.push(path);
		find_name_str = append_slash_at_path_end(path) + "*";
		find_dirs.pop();

		for(ret_val = file_find_helper::first(find_name_str.c_str(),find_data),ret = ret_val.first, find_handle = ret_val.second;
			ret!= false;
			ret = file_find_helper::next(find_handle,find_data).first)
		{
			if(file_find_helper::is_docs(find_data))
			{ continue;}		
			
			if(file_find_helper::is_direcrory(find_data))
			{	
				find_dirs.push(append_slash_at_path_end(path) + UPF_Wide_To_Ascii(find_data.cFileName).char_rep());
				continue;
			}
		}// for
		file_find_helper::close(find_handle);
		find_handle = UPF_INVALID_HANDLE;
	}// while

	while(!delete_dirs.empty())
	{
		if(UPF_OS::delete_empty_dir(delete_dirs.top().c_str())!=0)
			return false;
		delete_dirs.pop();
	}
	return true;

}

bool 
copy_dir(const char * dest_dir,const char * src_dir)
{

	string			find_name_str	= src_dir;
	string			dest_dir_str	= dest_dir;
	UPF_find_data_t	find_data		= {0};
	UPF_HANDLE		find_handle		= UPF_INVALID_HANDLE ;
	
	stack<string>	find_dirs;
	stack<string>	dest_dirs;	
	
	bool			ret				= false;
	file_find_helper::FD_TYPE		ret_val(false,UPF_INVALID_HANDLE);

	find_dirs.push(src_dir);
	dest_dirs.push(dest_dir);

	while(!find_dirs.empty())
	{
		memset(&find_data,0,sizeof(find_data));
		string path  = find_dirs.top();
		find_name_str = append_slash_at_path_end(path) + "*";
		find_dirs.pop();
		
		string dest_path  = dest_dirs.top();
		dest_dir_str	  = append_slash_at_path_end(dest_path);
		
		dest_dirs.pop();

		for(ret_val = file_find_helper::first(find_name_str.c_str(),find_data),ret = ret_val.first, find_handle = ret_val.second;
			ret!= false;
			ret = file_find_helper::next(find_handle,find_data).first)
		{
			if(file_find_helper::is_docs(find_data))
			{ continue;}		
			
			if(file_find_helper::is_direcrory(find_data))
			{	
				string dest_path_temp = (append_slash_at_path_end(dest_path) +UPF_Wide_To_Ascii(find_data.cFileName).char_rep());
				find_dirs.push(append_slash_at_path_end(path) + UPF_Wide_To_Ascii(find_data.cFileName).char_rep());
	
				if(UPF_OS::file_or_dir_is_exist(dest_path_temp.c_str())!=0)
				{
					if(UPF_OS::create_dir(dest_path_temp.c_str())!=0)
						return false;
				}
				dest_dirs.push(dest_path_temp);
				continue;
			}
			else
			{				
				string src_file		= (append_slash_at_path_end(path) + UPF_Wide_To_Ascii(find_data.cFileName).char_rep());
				string dest_file;	
		
				dest_file	= append_slash_at_path_end(dest_path)+UPF_Wide_To_Ascii(find_data.cFileName).char_rep();

				if(UPF_OS::copy_file(dest_file.c_str(),src_file.c_str())!=0)
				{
					file_find_helper::close(find_handle);
					return false;
				}
			}			
		}// for
			file_find_helper::close(find_handle);
	}// while

	return true;
}
}}