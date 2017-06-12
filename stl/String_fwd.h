#pragma once
#include "Config.hpp"
#include "String.h"
#include "Allocator.hpp"


typedef TinygGui::utils::basic_string<char, TinygGui::utils::malloc_alloc>    string;
typedef TinygGui::utils::basic_string<wchar_t, TinygGui::utils::malloc_alloc> wstring;

