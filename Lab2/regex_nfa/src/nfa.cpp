#include "../include/nfa.h"
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <queue>
#include <sstream>

namespace regexnfa {

NFA::NFA() : next_id(0), start(-1) {}

int NFA::new_state() { return next_id++; }

// void NFA::export_png(const std::string &filename) const {
//   std::string dotfile = filename + ".dot";
//   std::ofstream ofs(dotfile);
//   ofs << to_dot(); // Reuse your existing to_dot() function
//   ofs.close();
//
//   // Call Graphviz to generate PNG
//   std::string cmd = "dot -Tpng " + dotfile + " -o " + filename + ".png";
//   int ret = system(cmd.c_str());
//   if (ret != 0) {
//     std::cerr << "Error generating PNG file!" << std::endl;
//   } else {
//     std::cout << "NFA visualization saved as " << filename << ".png"
//               << std::endl;
//   }
// }
//
void NFA::add_transition(int from, char symbol, int to) {
  trans[from].push_back({symbol, to});
}
// Convert NFA to a debug string
std::string NFA::debug_dump() const {
  std::ostringstream oss;
  oss << "Start: S" << start << "  Accepts: ";
  for (auto a : accepts_set)
    oss << "S" << a << " ";
  oss << "\nTransitions:\n";
  std::vector<int> all;
  for (const auto &kv : trans) {
    all.push_back(kv.first);
    for (auto &t : kv.second)
      all.push_back(t.dest);
  }
  std::sort(all.begin(), all.end());
  all.erase(std::unique(all.begin(), all.end()), all.end());

  for (int s : all) {
    auto it = trans.find(s);
    if (it == trans.end())
      continue;
    for (auto &t : it->second) { // ← this line must be inside loop
      std::string lab = (t.symbol == EPSILON) ? "ε" : std::string(1, t.symbol);
      oss << "  S" << s << " -[" << lab << "]-> S" << t.dest << "\n";
    }
  }
  return oss.str();
}

// Convert NFA to dot format
std::string NFA::to_dot() const {
  std::ostringstream oss;
  oss << "digraph NFA {\n";
  oss << "  rankdir=LR;\n";
  oss << "  start [shape=point];\n";
  oss << "  start -> S" << start << ";\n";

  for (auto &kv : trans) {
    int s = kv.first;
    for (auto &t : kv.second) {
      std::string lbl = (t.symbol == EPSILON) ? "ε" : std::string(1, t.symbol);
      oss << "  S" << s << " -> S" << t.dest << " [label=\"" << lbl << "\"];\n";
    }
  }

  for (int a : accepts_set)
    oss << "  S" << a << " [shape=doublecircle];\n";

  oss << "}\n";
  return oss.str();
}

// Compute epsilon closure of a set of states
void NFA::epsilon_closure(const std::unordered_set<int> &src,
                          std::unordered_set<int> &out) const {
  std::queue<int> q;
  for (int s : src) {
    out.insert(s);
    q.push(s);
  }

  while (!q.empty()) {
    int v = q.front();
    q.pop();
    auto it = trans.find(v);
    if (it == trans.end())
      continue;
    for (auto &t : it->second) {
      if (t.symbol == EPSILON && out.insert(t.dest).second)
        q.push(t.dest);
    }
  }
}

// Move along a symbol from a set of states
void NFA::move(const std::unordered_set<int> &src, char symbol,
               std::unordered_set<int> &out) const {
  for (int s : src) {
    auto it = trans.find(s);
    if (it == trans.end())
      continue;
    for (auto &t : it->second) {
      if (t.symbol != EPSILON && t.symbol == symbol)
        out.insert(t.dest);
    }
  }
}

// Check if NFA accepts a string
bool NFA::accepts(const std::string &s) const {
  std::unordered_set<int> cur;
  epsilon_closure({start}, cur);

  for (char c : s) {
    std::unordered_set<int> next;
    move(cur, c, next);
    cur.clear();
    epsilon_closure(next, cur);
  }

  for (int a : accepts_set)
    if (cur.find(a) != cur.end())
      return true;

  return false;
}

} // namespace regexnfa
