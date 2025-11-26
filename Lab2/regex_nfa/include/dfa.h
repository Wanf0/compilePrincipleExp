#pragma once
#include <iostream>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace regexnfa {

struct DFAState {
  int id;
  std::set<int> nfa_states; // subset of NFA states
  bool is_accept = false;
  std::unordered_map<char, int> transitions; // symbol -> DFAState id
};

class DFA {
public:
  int start; // start state id
  std::vector<DFAState> states;

  void print() const;
  std::string to_dot() const;
};

} // namespace regexnfa
