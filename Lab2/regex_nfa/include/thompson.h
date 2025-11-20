
#pragma once
#include "nfa.h"
#include <string>

namespace regexnfa {

class ThompsonBuilder {
public:
    // Build NFA from a regex string
    static NFA build_from_regex(const std::string& regex);

private:
    // Basic NFA constructors
    static NFA symbol(char c);
    static NFA kleene_star(const NFA& frag);
    static NFA concatenate(const NFA& a, const NFA& b);
    static NFA alternate(const NFA& a, const NFA& b);

    // Convert regex to postfix notation
    static std::string regex_to_postfix(const std::string& regex);
};

} // namespace regexnfa
