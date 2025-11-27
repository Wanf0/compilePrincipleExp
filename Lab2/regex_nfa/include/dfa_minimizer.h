#ifndef DFA_MINIMIZER_H
#define DFA_MINIMIZER_H

#include "dfa.h"

namespace regexnfa {

class DFAMinimizer {
public:
  // Hopcroft
  static DFA minimize(const DFA &dfa);

private:
  // Find the partition
  static int find_partition(const std::vector<std::set<int>> &partitions,
                            int state);

  // Detact whether two states is equal
  static bool
  are_states_equivalent(const DFA &dfa, int state1, int state2,
                        const std::vector<std::set<int>> &partitions);
};

} // namespace regexnfa

#endif // DFA_MINIMIZER_H
