#include <iostream>
#include <string>

std::string trim(const std::string& str,
                 const std::string& whitespace = " \t");

std::string unescape_str(std::string& str);
std::string str_def(std::string& str);