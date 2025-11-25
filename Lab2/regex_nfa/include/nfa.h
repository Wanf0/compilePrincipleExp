#pragma once
#include <queue>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace regexnfa {

// internal epsilon label
static const char EPSILON = '\0';

// Represents a state in the NFA
struct State {
  int id;
  explicit State(int id_ = -1) : id(id_) {}
};

// Represents a transition in the NFA
struct Transition {
  char symbol; // EPSILON for epsilon transitions
  int dest;    // destination state id
  Transition(char s, int d) : symbol(s), dest(d) {}
};

// Non-deterministic Finite Automaton
class NFA {
public:
  NFA();

  // Low-level operations
  int new_state();
  void add_transition(int from, char symbol, int to);

  // Exporters
  std::string to_dot() const;
  std::string debug_dump() const;

  // Thompson helpers (used by builder)
  int start_state() const { return start; }
  const std::unordered_set<int> &accept_states() const { return accepts_set; }

  // Simulation
  bool accepts(const std::string &s) const;

  void export_png(const std::string &filename) const;

private:
  int next_id;                                            // next state id
  int start;                                              // start state id
  std::unordered_set<int> accepts_set;                    // accept states
  std::unordered_map<int, std::vector<Transition>> trans; // transitions

  // Simulation helpers
  void epsilon_closure(const std::unordered_set<int> &src,
                       std::unordered_set<int> &out) const;
  void move(const std::unordered_set<int> &src, char symbol,
            std::unordered_set<int> &out) const;

  friend class ThompsonBuilder;
  friend class NFAtoDFA;
};

} // namespace regexnfa
