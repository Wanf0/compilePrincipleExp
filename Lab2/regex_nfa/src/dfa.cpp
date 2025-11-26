#include "../include/dfa.h"
#include <fstream>

namespace regexnfa {

void DFA::print() const {
  std::cout << "\n--- DFA ---\n";
  std::cout << "Start state: " << start << "\n";
  for (const auto &s : states) {
    std::cout << "State " << s.id << " { ";
    for (int n : s.nfa_states)
      std::cout << n << " ";
    std::cout << "} ";
    if (s.is_accept)
      std::cout << "(ACCEPT)";
    std::cout << "\n";
    for (const auto &kv : s.transitions)
      std::cout << "   -" << kv.first << "-> " << kv.second << "\n";
  }
}

std::string DFA::to_dot() const {
  std::string dot = "digraph DFA {\n rankdir=LR;\n";
  dot += " start [shape=point];\n";
  dot += " start -> " + std::to_string(start) + ";\n";

  for (const auto &s : states) {
    dot += " " + std::to_string(s.id) + " [shape=";
    dot += s.is_accept ? "doublecircle" : "circle";
    dot += "];\n";
  }

  for (const auto &s : states) {
    for (const auto &kv : s.transitions) {
      dot += " " + std::to_string(s.id) + " -> " + std::to_string(kv.second);
      dot += " [label=\"" + std::string(1, kv.first) + "\"];\n";
    }
  }

  dot += "}\n";
  return dot;
}

} // namespace regexnfa
