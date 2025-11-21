
#include "dfa.h"
#include <iostream>

namespace regexnfa {

void DFA::print() const {
    std::cout << "DFA start state: " << start << "\n";
    for (const auto &s : states) {
        std::cout << "State " << s.id << " (accept=" << s.is_accept << ") : { ";
        for (int n : s.nfa_states) std::cout << n << " ";
        std::cout << "} \n";
        for (auto &kv : s.transitions)
            std::cout << "   -" << kv.first << "-> " << kv.second << "\n";
    }
}

bool DFA::accepts(const std::string &input) const {
    int cur = start;
    for (char c : input) {
        auto it = states[cur].transitions.find(c);
        if (it == states[cur].transitions.end())
            return false;
        cur = it->second;
    }
    return states[cur].is_accept;
}

} // namespace regexnfa
