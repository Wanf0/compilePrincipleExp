#include "../include/nfa_to_dfa.h"
#include "../include/thompson.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace regexnfa;

void print_dfa(const DFA &dfa) {
  std::cout << "\nDFA States:\n";
  for (const auto &s : dfa.states) {
    std::cout << "DFA State " << s.id << " { ";
    for (int n : s.nfa_states)
      std::cout << n << " ";
    std::cout << "} ";
    if (s.is_accept)
      std::cout << "(ACCEPT)";
    std::cout << "\n";
    for (const auto &kv : s.transitions)
      std::cout << "  -" << kv.first << "-> " << kv.second << "\n";
  }
}

void test_string(const NFA &nfa, const DFA &dfa, const std::string &str) {
  bool accept_nfa = nfa.accepts(str);
  bool accept_dfa = false;

  int current = dfa.start_state;
  for (char c : str) {
    if (dfa.states[current].transitions.count(c))
      current = dfa.states[current].transitions.at(c);
    else {
      current = -1;
      break;
    }
  }
  if (current != -1 && dfa.states[current].is_accept)
    accept_dfa = true;

  std::cout << "String \"" << str
            << "\": NFA=" << (accept_nfa ? "ACCEPT" : "REJECT")
            << ", DFA=" << (accept_dfa ? "ACCEPT" : "REJECT") << "\n";
}

void interactive_mode() {
  std::cout << "Regex → NFA/DFA interactive mode\n";
  std::cout << "Supports: *, +, ?, |, parentheses. Empty input to quit.\n";
  while (true) {
    std::cout << "regex> ";
    std::string regex;
    if (!std::getline(std::cin, regex) || regex.empty())
      break;

    try {
      NFA nfa = ThompsonBuilder::build_from_regex(regex);
      std::cout << "\nNFA built. Start= S" << nfa.start_state() << "\n";
      std::cout << nfa.debug_dump() << "\n";

      DFA dfa = NFAtoDFA::convert(nfa);
      print_dfa(dfa);

      std::string s;
      while (true) {
        std::cout << "\nEnter string to test (q to quit): ";
        std::getline(std::cin, s);
        if (s.empty() || s == "q")
          break;
        test_string(nfa, dfa, s);
      }

    } catch (const std::exception &e) {
      std::cerr << "Error: " << e.what() << "\n";
    }
  }
}

void batch_mode_from_file(const std::string &path) {
  std::ifstream ifs(path);
  if (!ifs) {
    std::cerr << "Cannot open file: " << path << "\n";
    return;
  }

  std::string line;
  while (std::getline(ifs, line)) {
    if (line.empty() || line[0] == '#')
      continue;

    std::istringstream iss(line);
    std::string regex, test, expected;
    std::getline(iss, regex, '\t');
    std::getline(iss, test, '\t');
    std::getline(iss, expected, '\t');

    try {
      NFA nfa = ThompsonBuilder::build_from_regex(regex);
      DFA dfa = NFAtoDFA::convert(nfa);
      test_string(nfa, dfa, test);
    } catch (const std::exception &e) {
      std::cerr << "Error building regex '" << regex << "': " << e.what()
                << "\n";
    }
  }
}

int main(int argc, char **argv) {
  if (argc == 1) {
    interactive_mode();
  } else if (argc == 2) {
    std::string arg = argv[1];
    if (arg == "--help" || arg == "-h") {
      std::cout << "Usage: regex_nfa_cli [tests_file.txt]\n"
                << "If no file provided, interactive mode runs.\n";
      return 0;
    }
    batch_mode_from_file(arg);
  } else {
    std::cerr << "Too many arguments\n";
    return 2;
  }
  return 0;
}
