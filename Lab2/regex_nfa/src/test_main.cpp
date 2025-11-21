
#include "nfa.h"
#include "nfa_to_dfa.h"
#include "thompson.h"
#include <iostream>
#include <string>

using namespace regexnfa;

int main() {
    ThompsonBuilder builder;
    std::string regex;

    std::cout << "Enter regex: ";
    std::cin >> regex;

    try {
        // Build NFA
        NFA nfa = builder.build(regex);
        std::cout << "\nNFA:\n" << nfa.debug_dump();

        // Convert to DFA
        DFA dfa = NFAtoDFA::convert(nfa);

        // Print DFA in readable form
        std::cout << "\nDFA:\n";
        for (auto &s : dfa.states) {
            std::cout << "DFA State " << s.id << " { ";
            for (int n : s.nfa_states) std::cout << n << " ";
            std::cout << "} ";
            if (s.is_accept) std::cout << "(ACCEPT)";
            std::cout << "\n";

            for (auto &kv : s.transitions) {
                std::cout << "  -" << kv.first << "-> " << kv.second << "\n";
            }
        }

        // Test strings
        std::string test;
        std::cout << "\nEnter string to test (q to quit): ";
        while (std::cin >> test) {
            if (test == "q") break;

            bool accept_nfa = nfa.accepts(test);
            bool accept_dfa = false;

            int current = dfa.start_state;
            for (char c : test) {
                if (current == -1) break;
                if (dfa.states[current].transitions.count(c))
                    current = dfa.states[current].transitions[c];
                else {
                    current = -1;
                    break;
                }
            }
            if (current != -1 && dfa.states[current].is_accept)
                accept_dfa = true;

            std::cout << "String \"" << test << "\": NFA=" << accept_nfa
                      << " DFA=" << accept_dfa << "\n";
            std::cout << "Next string: ";
        }

    } catch (std::runtime_error &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
