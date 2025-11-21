
#pragma once
#include "nfa.h"
#include <string>
#include <stack>

namespace regexnfa {

class ThompsonBuilder {
public:
    // Non-static wrapper for easier usage
    NFA build(const std::string &regex);

    // Build NFA from regex string (supports +, ?, *, |, concatenation)
    static NFA build_from_regex(const std::string &regex);

private:
    // Helpers for constructing NFAs
    static NFA symbol(char c);
    static NFA concatenate(const NFA &a, const NFA &b);
    static NFA alternate(const NFA &a, const NFA &b);
    static NFA kleene_star(const NFA &a);
    static NFA plus(const NFA &a);        // a+ = a.a*
    static NFA question(const NFA &a);    // a? = a|ε

    // Regex parsing helpers
    static std::string insert_concatenation(const std::string &regex);
    static std::string to_postfix(const std::string &regex);
};

} // namespace regexnfa
