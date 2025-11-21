
#include "nfa_to_dfa.h"
#include <queue>
#include <map>
#include <set>
#include <algorithm>

namespace regexnfa {

// Compute epsilon closure of a set of NFA states
std::set<int> NFAtoDFA::epsilon_closure(const NFA& nfa, const std::set<int>& states) {
    std::set<int> closure = states;
    std::queue<int> q;
    for (int s : states) q.push(s);

    while (!q.empty()) {
        int v = q.front(); q.pop();
        auto it = nfa.trans.find(v); // friend access
        if (it == nfa.trans.end()) continue;
        for (auto &t : it->second) {
            if (t.symbol == EPSILON && closure.insert(t.dest).second)
                q.push(t.dest);
        }
    }
    return closure;
}

// Move on a symbol
std::set<int> NFAtoDFA::move_on_symbol(const NFA& nfa, const std::set<int>& states, char c) {
    std::set<int> result;
    for (int s : states) {
        auto it = nfa.trans.find(s);
        if (it == nfa.trans.end()) continue;
        for (auto &t : it->second) {
            if (t.symbol != EPSILON && t.symbol == c)
                result.insert(t.dest);
        }
    }
    return result;
}

// NFA → DFA using subset construction
DFA NFAtoDFA::convert(const NFA& nfa) {
    DFA dfa;
    std::vector<DFAState> dfa_states;
    std::map<std::set<int>, int> state_map; // NFA set -> DFA id
    int next_dfa_id = 0;

    std::set<int> start_set = epsilon_closure(nfa, {nfa.start_state()});
    state_map[start_set] = next_dfa_id;

    DFAState start_state;
    start_state.id = next_dfa_id++;
    start_state.nfa_states = start_set;
    start_state.is_accept = false;
    for (int s : start_set)
        if (nfa.accept_states().count(s))
            start_state.is_accept = true;

    dfa_states.push_back(start_state);
    dfa.start_state = start_state.id;

    std::queue<int> q;
    q.push(start_state.id);

    while (!q.empty()) {
        int current_id = q.front(); q.pop();
        DFAState &current = dfa_states[current_id];

        std::set<char> symbols;
        for (int s : current.nfa_states) {
            auto it = nfa.trans.find(s);
            if (it == nfa.trans.end()) continue;
            for (auto &t : it->second) {
                if (t.symbol != EPSILON)
                    symbols.insert(t.symbol);
            }
        }

        for (char sym : symbols) {
            std::set<int> moved = move_on_symbol(nfa, current.nfa_states, sym);
            std::set<int> next_set = epsilon_closure(nfa, moved);

            if (state_map.find(next_set) == state_map.end()) {
                DFAState next_state;
                next_state.id = next_dfa_id++;
                next_state.nfa_states = next_set;
                next_state.is_accept = false;
                for (int s : next_set)
                    if (nfa.accept_states().count(s))
                        next_state.is_accept = true;

                dfa_states.push_back(next_state);
                state_map[next_set] = next_state.id;
                q.push(next_state.id);
            }

            current.transitions[sym] = state_map[next_set];
        }
    }

    dfa.states = dfa_states;
    return dfa;
}

} // namespace regexnfa
