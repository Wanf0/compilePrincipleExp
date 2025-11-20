
#include "thompson.h"
#include <stack>
#include <stdexcept>

namespace regexnfa {

// --- Helper constructors ---
NFA ThompsonBuilder::symbol(char c) {
    NFA nfa;
    int s = nfa.new_state();
    int f = nfa.new_state();
    nfa.add_transition(s, c, f);
    nfa.start = s;
    nfa.accepts_set.insert(f);
    return nfa;
}

NFA ThompsonBuilder::kleene_star(const NFA& frag) {
    NFA out;
    int s = out.new_state();
    int f = out.new_state();
    // Copy all states and transitions
    for (auto& kv : frag.trans) {
        for (const auto& t : kv.second) {
            out.trans[kv.first].push_back({t.symbol, t.dest});
        }
    }
    for (int a : frag.accepts_set) {
        out.trans[a].push_back({EPSILON, frag.start});
        out.trans[a].push_back({EPSILON, f});
    }
    out.trans[s].push_back({EPSILON, frag.start});
    out.trans[s].push_back({EPSILON, f});
    out.start = s;
    out.accepts_set.insert(f);
    return out;
}

NFA ThompsonBuilder::concatenate(const NFA& a, const NFA& b) {
    NFA out;
    // Copy a
    for (auto& kv : a.trans) {
        for (const auto& t : kv.second) out.trans[kv.first].push_back({t.symbol, t.dest});
    }
    // Copy b
    for (auto& kv : b.trans) {
        for (const auto& t : kv.second) out.trans[kv.first].push_back({t.symbol, t.dest});
    }
    // Connect a's accept states to b's start
    for (int x : a.accepts_set) {
        out.trans[x].push_back({EPSILON, b.start_state()});
    }
    out.start = a.start_state();
    out.accepts_set = b.accept_states();
    return out;
}

NFA ThompsonBuilder::alternate(const NFA& a, const NFA& b) {
    NFA out;
    int s = out.new_state();
    int f = out.new_state();
    // Copy a
    for (auto& kv : a.trans)
        for (const auto& t : kv.second)
            out.trans[kv.first].push_back({t.symbol, t.dest});
    // Copy b
    for (auto& kv : b.trans)
        for (const auto& t : kv.second)
            out.trans[kv.first].push_back({t.symbol, t.dest});
    // Connect new start to both
    out.trans[s].push_back({EPSILON, a.start_state()});
    out.trans[s].push_back({EPSILON, b.start_state()});
    // Connect accepts to new final state
    for (int x : a.accept_states()) out.trans[x].push_back({EPSILON, f});
    for (int x : b.accept_states()) out.trans[x].push_back({EPSILON, f});
    out.start = s;
    out.accepts_set.insert(f);
    return out;
}

// --- Convert infix regex to postfix ---
std::string ThompsonBuilder::regex_to_postfix(const std::string& regex) {
    // Simple Shunting Yard implementation (assumes valid regex)
    std::string out;
    std::stack<char> st;
    for (char c : regex) {
        if (isalnum(c)) out += c;
        else if (c == '(') st.push(c);
        else if (c == ')') {
            while (!st.empty() && st.top() != '(') {
                out += st.top();
                st.pop();
            }
            st.pop(); // pop '('
        } else {
            while (!st.empty() && st.top() != '(') {
                out += st.top();
                st.pop();
            }
            st.push(c);
        }
    }
    while (!st.empty()) {
        out += st.top();
        st.pop();
    }
    return out;
}

// --- Build from regex ---
NFA ThompsonBuilder::build_from_regex(const std::string& regex) {
    std::string postfix = regex_to_postfix(regex);
    std::stack<NFA> st;
    for (char ch : postfix) {
        if (isalnum(ch)) st.push(symbol(ch));
        else if (ch == '*') {
            NFA frag = st.top(); st.pop();
            st.push(kleene_star(frag));
        } else if (ch == '.') {
            NFA b = st.top(); st.pop();
            NFA a = st.top(); st.pop();
            st.push(concatenate(a, b));
        } else if (ch == '|') {
            NFA b = st.top(); st.pop();
            NFA a = st.top(); st.pop();
            st.push(alternate(a, b));
        } else {
            throw std::runtime_error("Unknown operator in regex");
        }
    }
    if (st.size() != 1) throw std::runtime_error("Invalid regex");
    return st.top();
}

} // namespace regexnfa
