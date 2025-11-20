
#include <iostream>
#include "nfa.h"
#include "thompson.h"

using namespace regexnfa;

int main() {
    // Example regex
    std::string regex = "a(b|c)*";

    // Build NFA from regex
    NFA nfa = ThompsonBuilder::build_from_regex(regex);

    // Show the NFA as DOT (can use Graphviz to visualize)
    std::cout << "DOT representation of NFA:" << std::endl;
    std::cout << nfa.to_dot() << std::endl;

    // Test some strings
    std::string test1 = "abcb";
    std::string test2 = "accc";
    std::string test3 = "abcx";

    std::cout << test1 << " accepted? " << (nfa.accepts(test1) ? "Yes" : "No") << std::endl;
    std::cout << test2 << " accepted? " << (nfa.accepts(test2) ? "Yes" : "No") << std::endl;
    std::cout << test3 << " accepted? " << (nfa.accepts(test3) ? "Yes" : "No") << std::endl;

    return 0;
}
