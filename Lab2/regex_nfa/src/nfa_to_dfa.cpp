#include "../include/nfa_to_dfa.h"
#include <map>
#include <queue>

namespace regexnfa {

std::set<int> NFAtoDFA::epsilon_closure(const NFA &nfa,
                                        const std::set<int> &states) {
  std::set<int> closure = states;
  std::queue<int> q;
  for (int s : states)
    q.push(s);

  while (!q.empty()) {
    int u = q.front();
    q.pop();
    auto it = nfa.trans.find(u);
    if (it == nfa.trans.end())
      continue;
    for (auto &t : it->second) {
      if (t.symbol == EPSILON && closure.insert(t.dest).second)
        q.push(t.dest);
    }
  }
  return closure;
}

std::set<int> NFAtoDFA::move_on_symbol(const NFA &nfa,
                                       const std::set<int> &states,
                                       char symbol) {
  std::set<int> result;
  for (int s : states) {
    auto it = nfa.trans.find(s);
    if (it == nfa.trans.end())
      continue;
    for (auto &t : it->second) {
      if (t.symbol != EPSILON && t.symbol == symbol)
        result.insert(t.dest);
    }
  }
  return result;
}

DFA NFAtoDFA::convert(const NFA &nfa) {
  DFA dfa;
  std::vector<DFAState> dfa_states;
  std::map<std::set<int>, int> state_map;
  int next_id = 0;

  // 获取所有可能的输入符号（除了epsilon）
  std::set<char> alphabet;
  for (const auto &state_trans : nfa.trans) {
    for (const auto &transition : state_trans.second) {
      if (transition.symbol != EPSILON) {
        alphabet.insert(transition.symbol);
      }
    }
  }

  // 创建起始状态
  std::set<int> start_set = epsilon_closure(nfa, {nfa.start_state()});
  DFAState start_state;
  start_state.id = next_id++;
  start_state.nfa_states = start_set;
  start_state.is_accept = false;
  for (int s : start_set) {
    if (nfa.accept_states().count(s)) {
      start_state.is_accept = true;
      break;
    }
  }

  dfa.start = start_state.id;
  dfa_states.push_back(start_state);
  state_map[start_set] = start_state.id;

  std::queue<int> q;
  q.push(start_state.id);

  while (!q.empty()) {
    int cur_id = q.front();
    q.pop();

    // 不要使用引用，直接通过索引访问
    // DFAState &cur_state = dfa_states[cur_id]; // 这行会导致问题

    // 对字母表中的每个符号进行处理
    for (char sym : alphabet) {
      std::set<int> move_set =
          move_on_symbol(nfa, dfa_states[cur_id].nfa_states, sym);
      if (move_set.empty()) {
        continue;
      }

      std::set<int> next_set = epsilon_closure(nfa, move_set);
      if (next_set.empty()) {
        continue;
      }

      int target_state_id;
      if (state_map.find(next_set) == state_map.end()) {
        // 新状态
        DFAState new_state;
        new_state.id = next_id++;
        new_state.nfa_states = next_set;
        new_state.is_accept = false;
        for (int s : next_set) {
          if (nfa.accept_states().count(s)) {
            new_state.is_accept = true;
            break;
          }
        }

        dfa_states.push_back(new_state);
        state_map[next_set] = new_state.id;
        q.push(new_state.id);
        target_state_id = new_state.id;
      } else {
        target_state_id = state_map[next_set];
      }

      // 直接通过索引添加转换，避免使用引用
      dfa_states[cur_id].transitions[sym] = target_state_id;
    }
  }

  dfa.states = dfa_states;
  return dfa;
}
} // namespace regexnfa
