#include "../include/dfa_minimizer.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <queue>
#include <set>

namespace regexnfa {

DFA DFAMinimizer::minimize(const DFA &dfa) {
  if (dfa.states.empty())
    return dfa;

  std::cout << "Starting DFA minimization...\n";
  std::cout << "Original DFA has " << dfa.states.size() << " states\n";

  size_t N = dfa.states.size();

  // 构建全局字母表
  std::set<char> global_alphabet;
  for (const auto &state : dfa.states) {
    for (const auto &t : state.transitions) {
      global_alphabet.insert(t.first);
    }
  }

  // 步骤1: 初始化分区（接受状态和非接受状态）
  std::vector<std::set<int>> partitions(2);
  for (size_t i = 0; i < N; ++i) {
    if (dfa.states[i].is_accept)
      partitions[0].insert(i);
    else
      partitions[1].insert(i);
  }
  // 删除空分区
  partitions.erase(
      std::remove_if(partitions.begin(), partitions.end(),
                     [](const std::set<int> &s) { return s.empty(); }),
      partitions.end());

  std::cout << "Initial partitions: " << partitions.size() << "\n";

  // 建立 state -> partition 映射
  std::vector<int> state_to_partition(N);
  for (size_t i = 0; i < partitions.size(); ++i)
    for (int s : partitions[i])
      state_to_partition[s] = i;

  // 步骤2: 细化分区
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

      // 根据行为对状态分组
      std::map<std::vector<int>, std::set<int>> behavior_groups;

      for (int s : P) {
        std::vector<int> behavior;
        for (char sym : global_alphabet) {
          auto it = dfa.states[s].transitions.find(sym);
          if (it != dfa.states[s].transitions.end()) {
            int target = it->second;
            behavior.push_back(state_to_partition[target]);
          } else {
            behavior.push_back(-1);
          }
        }
        behavior_groups[behavior].insert(s);
      }

      for (const auto &grp : behavior_groups) {
        new_partitions.push_back(grp.second);
        if (grp.second.size() < P.size())
          changed = true;
      }
    }

    partitions = new_partitions;

    // 更新 state -> partition 映射
    for (size_t i = 0; i < partitions.size(); ++i)
      for (int s : partitions[i])
        state_to_partition[s] = i;

    std::cout << "Iteration " << iteration << ": " << partitions.size()
              << " partitions\n";
  }

  std::cout << "Final number of partitions: " << partitions.size() << "\n";

  // 步骤3: 构建最小化 DFA
  DFA minimized_dfa;
  minimized_dfa.states.resize(partitions.size());

  for (size_t i = 0; i < partitions.size(); ++i) {
    DFAState new_state;
    new_state.id = i;
    // 合并 NFA 状态
    for (int old_s : partitions[i])
      new_state.nfa_states.insert(dfa.states[old_s].nfa_states.begin(),
                                  dfa.states[old_s].nfa_states.end());

    // 是否为接受状态
    new_state.is_accept = false;
    for (int old_s : partitions[i]) {
      if (dfa.states[old_s].is_accept) {
        new_state.is_accept = true;
        break;
      }
    }

    // 转移关系：取第一个状态作为代表
    int representative = *partitions[i].begin();
    for (const auto &t : dfa.states[representative].transitions) {
      char sym = t.first;
      int old_target = t.second;
      int new_target = state_to_partition[old_target];
      new_state.transitions[sym] = new_target;
    }

    minimized_dfa.states[i] = new_state;
  }

  // 起始状态
  minimized_dfa.start = state_to_partition[dfa.start];

  std::cout << "Minimized DFA has " << minimized_dfa.states.size()
            << " states\n";

  return minimized_dfa;
}

} // namespace regexnfa
