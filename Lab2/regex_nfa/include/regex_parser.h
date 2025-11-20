#pragma once
#include <string>


namespace regexnfa {


// Convert infix regex (supports: | concatenation implicit, * )
// into a form with explicit concatenation '.' and then postfix.
std::string add_concat(const std::string& regex);
std::string to_postfix(const std::string& regex_with_concat);


} // namespace regexnfa
