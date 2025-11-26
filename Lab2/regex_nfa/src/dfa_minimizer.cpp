#include "../include/dfa_minimizer.h"
#include <algorithm>
#include <map>
#include <queue>
#include <set>

namespace regexnfa {

DFA DFAMinimizer::minimize(const DFA &dfa) {
  if (dfa.states.empty()) {
    return dfa;
  }

  std::cout << "Starting DFA minimization...\n";
  std::cout << "Original DFA has " << dfa.states.size() << " states\n";

  // 步骤1: 初始化分割 - 将状态分为接受状态和非接受状态
  std::vector<std::set<int>> partitions;
  std::set<int> accepting_states;
  std::set<int> non_accepting_states;

  for (const auto &state : dfa.states) {
    if (state.is_accept) {
      accepting_states.insert(state.id);
    } else {
      non_accepting_states.insert(state.id);
    }
  }

  if (!accepting_states.empty()) {
    partitions.push_back(accepting_states);
  }
  if (!non_accepting_states.empty()) {
    partitions.push_back(non_accepting_states);
  }

  std::cout << "Initial partitions: " << partitions.size()
            << " (Accepting: " << accepting_states.size()
            << ", Non-accepting: " << non_accepting_states.size() << ")\n";

  // 步骤2: 细化分割 - 使用Hopcroft算法
  bool changed = true;
  int iteration = 0;

  while (changed) {
    changed = false;
    iteration++;
    std::vector<std::set<int>> new_partitions;

    for (const auto &P : partitions) {
      if (P.size() <= 1) {
        new_partitions.push_back(P);
        continue;
      }

      // 获取所有输入符号
      std::set<char> alphabet;
      for (int state_id : P) {
        const auto &state = dfa.states[state_id];
        for (const auto &trans : state.transitions) {
          alphabet.insert(trans.first);
        }
      }

      // 根据转移行为对状态进行分组
      std::map<std::vector<int>, std::set<int>> behavior_groups;

      for (int state_id : P) {
        const auto &state = dfa.states[state_id];
        std::vector<int> behavior;

        // 对于每个符号，记录转移目标所在的分区
        for (char sym : alphabet) {
          auto it = state.transitions.find(sym);
          if (it != state.transitions.end()) {
            int target_state = it->second;
            int partition_index = find_partition(partitions, target_state);
            behavior.push_back(partition_index);
          } else {
            behavior.push_back(-1); // 表示没有转移
          }
        }

        behavior_groups[behavior].insert(state_id);
      }

      // 将行为相同的状态放在同一个分区
      for (const auto &group : behavior_groups) {
        new_partitions.push_back(group.second);
        if (group.second.size() < P.size()) {
          changed = true;
        }
      }
    }

    partitions = new_partitions;
    std::cout << "Iteration " << iteration << ": " << partitions.size()
              << " partitions\n";
  }

  std::cout << "Final number of partitions: " << partitions.size() << "\n";

  // 步骤3: 构建最小化DFA
  DFA minimized_dfa;

  // 创建状态到新分区ID的映射
  std::map<int, int> state_to_new_id;
  for (size_t i = 0; i < partitions.size(); ++i) {
    for (int state_id : partitions[i]) {
      state_to_new_id[state_id] = i;
    }
  }

  // 构建新状态
  for (size_t i = 0; i < partitions.size(); ++i) {
    DFAState new_state;
    new_state.id = i;

    // 合并原NFA状态集合
    for (int old_state_id : partitions[i]) {
      const auto &old_state = dfa.states[old_state_id];
      new_state.nfa_states.insert(old_state.nfa_states.begin(),
                                  old_state.nfa_states.end());
    }

    // 确定是否为接受状态（只要分区中有一个接受状态，新状态就是接受状态）
    new_state.is_accept = false;
    for (int old_state_id : partitions[i]) {
      if (dfa.states[old_state_id].is_accept) {
        new_state.is_accept = true;
        break;
      }
    }

    // 构建转移关系 - 使用分区中的第一个状态作为代表
    int representative = *partitions[i].begin();
    const auto &rep_state = dfa.states[representative];

    for (const auto &trans : rep_state.transitions) {
      char symbol = trans.first;
      int old_target = trans.second;
      int new_target = state_to_new_id[old_target];
      new_state.transitions[symbol] = new_target;
    }

    minimized_dfa.states.push_back(new_state);
  }

  // 设置起始状态
  minimized_dfa.start = state_to_new_id[dfa.start];

  std::cout << "Minimized DFA has " << minimized_dfa.states.size()
            << " states\n";

  return minimized_dfa;
}

int DFAMinimizer::find_partition(const std::vector<std::set<int>> &partitions,
                                 int state) {
  for (size_t i = 0; i < partitions.size(); ++i) {
    if (partitions[i].count(state)) {
      return i;
    }
  }
  return -1; // 未找到
}

bool DFAMinimizer::are_states_equivalent(
    const DFA &dfa, int state1, int state2,
    const std::vector<std::set<int>> &partitions) {
  const auto &s1 = dfa.states[state1];
  const auto &s2 = dfa.states[state2];

  // 如果一个是接受状态而另一个不是，它们不等价
  if (s1.is_accept != s2.is_accept) {
    return false;
  }

  // 获取所有可能的输入符号
  std::set<char> symbols;
  for (const auto &trans : s1.transitions) {
    symbols.insert(trans.first);
  }
  for (const auto &trans : s2.transitions) {
    symbols.insert(trans.first);
  }

  // 检查每个符号的转移是否指向相同的分区
  for (char sym : symbols) {
    auto it1 = s1.transitions.find(sym);
    auto it2 = s2.transitions.find(sym);

    int target1 = (it1 != s1.transitions.end()) ? it1->second : -1;
    int target2 = (it2 != s2.transitions.end()) ? it2->second : -1;

    // 如果两个状态在某个符号上都没有转移，继续检查下一个符号
    if (target1 == -1 && target2 == -1) {
      continue;
    }

    // 如果一个有转移而另一个没有，它们不等价
    if ((target1 == -1) != (target2 == -1)) {
      return false;
    }

    // 检查转移目标是否在同一个分区
    int partition1 = find_partition(partitions, target1);
    int partition2 = find_partition(partitions, target2);

    if (partition1 != partition2) {
      return false;
    }
  }

  return true;
}

} // namespace regexnfa
