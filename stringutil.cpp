
#include <algorithm>
#include <string>

#include "stringutil.h"

/**
 * strip quote dobel dari parser.
 */
std::string trim(const std::string& str,
                 const std::string& whitespace)
{
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}


/**
 * Unescape raw '\n' into new line char.
 */
std::string unescape_str(std::string& str){

    for (size_t pos = 0; ; pos += 2) {
        pos = str.find("\\n", pos);
        if (pos == std::string::npos)
            break;
        str.erase(pos, 2);
        str.insert(pos, "\n");
    }
    
    return str;
}

std::string str_def(std::string& a){
    std::string b = trim(a, " \t\"");
    return unescape_str(b);
}