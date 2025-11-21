
#pragma once
#include "nfa.h"
#include <set>
#include <map>
#include <vector>
#include <iostream>

namespace regexnfa {

// DFA state
struct DFAState {
    int id;
    std::set<int> nfa_states;
    bool is_accept = false;
    std::map<char, int> transitions; // symbol -> DFA state id
};

// DFA itself
struct DFA {
    int start_state;
    std::vector<DFAState> states;

    void print() const {
        std::cout << "DFA start state: " << start_state << "\n";
        for (auto &s : states) {
            std::cout << "State " << s.id << " { ";
            for (int n : s.nfa_states) std::cout << n << " ";
            if (s.is_accept) std::cout << "(ACCEPT)";
            std::cout << " }\n";

            for (auto &kv : s.transitions)
                std::cout << "  " << kv.first << " -> " << kv.second << "\n";
        }
    }
};

// Converts an NFA to DFA
class NFAtoDFA {
public:
    static DFA convert(const NFA& nfa);

private:
    static std::set<int> epsilon_closure(const NFA& nfa, const std::set<int>& states);
    static std::set<int> move_on_symbol(const NFA& nfa, const std::set<int>& states, char c);
};

} // namespace regexnfa
