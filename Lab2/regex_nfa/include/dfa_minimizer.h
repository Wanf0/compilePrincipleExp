#ifndef DFA_MINIMIZER_H
#define DFA_MINIMIZER_H

#include "dfa.h"

namespace regexnfa {

class DFAMinimizer {
public:
  // 使用Hopcroft算法最小化DFA
  static DFA minimize(const DFA &dfa);

private:
  // 辅助函数：找到状态所在的分区索引
  static int find_partition(const std::vector<std::set<int>> &partitions,
                            int state);

  // 辅助函数：检查两个状态在给定分区下是否等价
  static bool
  are_states_equivalent(const DFA &dfa, int state1, int state2,
                        const std::vector<std::set<int>> &partitions);
};

} // namespace regexnfa

#endif // DFA_MINIMIZER_H
