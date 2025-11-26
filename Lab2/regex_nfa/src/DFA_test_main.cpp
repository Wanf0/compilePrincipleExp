#include "../include/nfa_to_dfa.h"
#include "../include/thompson.h"
#include <fstream>
#include <iostream>
#include <string>

using namespace regexnfa;

int main() {
  std::cout << "Enter regex: ";
  std::string regex;
  std::getline(std::cin, regex);

  try {
    NFA nfa = ThompsonBuilder::build_from_regex(regex);
    std::cout << "\n--- NFA ---\n";
    std::cout << nfa.debug_dump() << "\n";

    DFA dfa = NFAtoDFA::convert(nfa);
    dfa.print();

    std::string dot_file = "dfa.dot";
    std::ofstream ofs(dot_file);
    ofs << dfa.to_dot();
    ofs.close();
    std::cout << "\nDFA DOT saved to " << dot_file << "\n";
    std::cout << "Generate image: dot -Tpng dfa.dot -o dfa.png\n";

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
  }

  return 0;
}
