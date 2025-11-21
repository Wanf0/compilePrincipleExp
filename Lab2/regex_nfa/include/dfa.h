
#pragma once
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <vector>
#include <iostream>

namespace regexnfa {

// DFA State
struct DFAState {
    int id;  // sequential id
    std::set<int> nfa_states;  // subset of NFA states
    bool is_accept = false;
    std::unordered_map<char, int> transitions;  // symbol -> DFAState id
};

// Deterministic Finite Automaton
class DFA {
public:
    int start;
    std::vector<DFAState> states;

    void print() const;
    bool accepts(const std::string &s) const;
};

} // namespace regexnfa
