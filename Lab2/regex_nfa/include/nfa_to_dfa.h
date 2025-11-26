#pragma once
#include "dfa.h"
#include "nfa.h"
#include <set>

namespace regexnfa {

class NFAtoDFA {
public:
  static DFA convert(const NFA &nfa);

private:
  static std::set<int> epsilon_closure(const NFA &nfa,
                                       const std::set<int> &states);
  static std::set<int> move_on_symbol(const NFA &nfa,
                                      const std::set<int> &states, char symbol);
};

} // namespace regexnfa
