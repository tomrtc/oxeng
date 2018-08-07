#pragma once


#include <string>




bool directory_exists(const char *path);
bool file_exists(const char *path);
std::string get_file_contents(const std::string filename);
