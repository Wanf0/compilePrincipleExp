#include "../include/thompson.h"
#include <cctype>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>

namespace regexnfa {

// ---------- Public function ----------
NFA ThompsonBuilder::build_from_regex(const std::string &regex) {
  std::string expanded =
      insert_concatenation(regex); // insert '.' automatically
  std::cout << expanded << std::endl;
  std::string postfix = to_postfix(expanded);

  std::stack<NFA> st;

  for (char ch : postfix) {
    switch (ch) {
    case '.': {
      NFA b = st.top();
      st.pop();
      NFA a = st.top();
      st.pop();
      st.push(concatenate(a, b));
      break;
    }
    case '|': {
      NFA b = st.top();
      st.pop();
      NFA a = st.top();
      st.pop();
      st.push(alternate(a, b));
      break;
    }
    case '*': {
      NFA a = st.top();
      st.pop();
      st.push(kleene_star(a));
      break;
    }
    case '+': {
      NFA a = st.top();
      st.pop();
      st.push(plus(a));
      break;
    }
    case '?': {
      NFA a = st.top();
      st.pop();
      st.push(question(a));
      break;
    }
    default: {
      st.push(symbol(ch));
      break;
    }
    }
  }

  if (st.size() != 1)
    throw std::runtime_error("Invalid regex");
  return st.top();
}

NFA ThompsonBuilder::build(const std::string &regex) {
  return build_from_regex(regex);
}

// ---------- Helper NFA builders ----------
NFA ThompsonBuilder::symbol(char c) {
  NFA nfa;
  int s = nfa.new_state();
  int f = nfa.new_state();
  nfa.start = s;
  nfa.accepts_set.insert(f);
  nfa.add_transition(s, c, f);
  return nfa;
}

NFA ThompsonBuilder::concatenate(const NFA &a, const NFA &b) {
  NFA out = a; // copy all of a

  // remap all states of b
  std::set<int> states_b;
  for (const auto &[from, vec] : b.trans) {
    states_b.insert(from);
    for (auto &t : vec)
      states_b.insert(t.dest);
  }
  states_b.insert(b.start);
  for (int fb : b.accepts_set)
    states_b.insert(fb);

  std::map<int, int> remap_b;
  for (int st : states_b)
    remap_b[st] = out.new_state(); // new state IDs in out

  // copy transitions from b with remapped IDs
  for (const auto &[from, vec] : b.trans) {
    int new_from = remap_b[from];
    for (const auto &t : vec)
      out.add_transition(new_from, t.symbol, remap_b.at(t.dest));
  }

  // link a's accept states to b's start
  for (int fa : a.accepts_set)
    out.add_transition(fa, EPSILON, remap_b.at(b.start));

  // new accept states = remapped b accepts
  out.accepts_set.clear();
  for (int fb : b.accepts_set)
    out.accepts_set.insert(remap_b.at(fb));

  return out;
}

NFA ThompsonBuilder::alternate(const NFA &a, const NFA &b) {
  NFA out;
  int s = out.new_state(); // new start
  int f = out.new_state(); // new accept

  // --- remap a ---
  std::set<int> states_a;
  for (const auto &[from, vec] : a.trans) {
    states_a.insert(from);
    for (auto &t : vec)
      states_a.insert(t.dest);
  }
  states_a.insert(a.start);
  for (int fa : a.accepts_set)
    states_a.insert(fa);

  std::map<int, int> remap_a;
  for (int st : states_a)
    remap_a[st] = out.new_state();

  for (const auto &[from, vec] : a.trans) {
    int new_from = remap_a[from];
    for (const auto &t : vec)
      out.add_transition(new_from, t.symbol, remap_a.at(t.dest));
  }

  // --- remap b ---
  std::set<int> states_b;
  for (const auto &[from, vec] : b.trans) {
    states_b.insert(from);
    for (auto &t : vec)
      states_b.insert(t.dest);
  }
  states_b.insert(b.start);
  for (int fb : b.accepts_set)
    states_b.insert(fb);

  std::map<int, int> remap_b;
  for (int st : states_b)
    remap_b[st] = out.new_state();

  for (const auto &[from, vec] : b.trans) {
    int new_from = remap_b[from];
    for (const auto &t : vec)
      out.add_transition(new_from, t.symbol, remap_b.at(t.dest));
  }

  // connect new start to both a and b
  out.add_transition(s, EPSILON, remap_a.at(a.start));
  out.add_transition(s, EPSILON, remap_b.at(b.start));

  // connect both a and b accepts to new accept
  for (int fa : a.accepts_set)
    out.add_transition(remap_a.at(fa), EPSILON, f);
  for (int fb : b.accepts_set)
    out.add_transition(remap_b.at(fb), EPSILON, f);

  out.start = s;
  out.accepts_set.insert(f);
  return out;
}

NFA ThompsonBuilder::kleene_star(const NFA &a) {
  NFA out;
  int s = out.new_state(); // new start
  int f = out.new_state(); // new accept

  // collect all states of a (including start and accept states)
  std::set<int> states;
  for (const auto &[from, vec] : a.trans) {
    states.insert(from);
    for (auto &t : vec)
      states.insert(t.dest);
  }
  states.insert(a.start);
  for (int fa : a.accepts_set)
    states.insert(fa);

  // remap all states to fresh ones
  std::map<int, int> remap;
  for (int st : states)
    remap[st] = out.new_state();

  // copy transitions
  for (const auto &[from, vec] : a.trans) {
    int new_from = remap[from];
    for (const auto &t : vec)
      out.add_transition(new_from, t.symbol, remap.at(t.dest));
  }

  // connect new start and new accept
  out.add_transition(s, EPSILON, remap.at(a.start)); // start -> a
  out.add_transition(s, EPSILON, f);                 // start -> accept
  for (int fa : a.accepts_set) {
    out.add_transition(remap.at(fa), EPSILON, remap.at(a.start)); // loop back
    out.add_transition(remap.at(fa), EPSILON, f); // end -> accept
  }

  out.start = s;
  out.accepts_set.insert(f);
  return out;
}

NFA ThompsonBuilder::plus(const NFA &a) {
  NFA star = kleene_star(a);
  return concatenate(a, star);
}

NFA ThompsonBuilder::question(const NFA &a) {
  NFA empty;
  int s = empty.new_state();
  int f = empty.new_state();
  empty.start = s;
  empty.accepts_set.insert(f);
  empty.add_transition(s, EPSILON, f);
  return alternate(a, empty);
}

// ---------- Regex parsing helpers ----------
bool is_operator(char c) {
  return c == '*' || c == '+' || c == '?' || c == '|' || c == '.' || c == '(' ||
         c == ')';
}

int precedence(char op) {
  switch (op) {
  case '*':
  case '+':
  case '?':
    return 3;
  case '.':
    return 2;
  case '|':
    return 1;
  default:
    return 0;
  }
}

bool is_right_associative(char op) {
  return op == '*' || op == '+' || op == '?';
}

// Convert infix to postfix using shunting-yard
std::string ThompsonBuilder::to_postfix(const std::string &regex) {
  std::string out;
  std::stack<char> st;

  for (char c : regex) {
    if (!is_operator(c)) {
      out += c;
    } else if (c == '(') {
      st.push(c);
    } else if (c == ')') {
      while (!st.empty() && st.top() != '(') {
        out += st.top();
        st.pop();
      }
      if (!st.empty())
        st.pop();
    } else {
      while (!st.empty() && ((is_right_associative(c) &&
                              precedence(st.top()) > precedence(c)) ||
                             (!is_right_associative(c) &&
                              precedence(st.top()) >= precedence(c)))) {
        out += st.top();
        st.pop();
      }
      st.push(c);
    }
  }

  while (!st.empty()) {
    out += st.top();
    st.pop();
  }
  std::cout << out << std::endl;
  return out;
}

// Insert explicit concatenation operator '.'
std::string ThompsonBuilder::insert_concatenation(const std::string &regex) {
  std::string out;
  for (size_t i = 0; i < regex.size(); ++i) {
    char c1 = regex[i];
    out += c1;
    if (i + 1 < regex.size()) {
      char c2 = regex[i + 1];
      if ((c1 == ')' || c1 == '*' || c1 == '+' || c1 == '?' ||
           !is_operator(c1)) &&
          (c2 == '(' || !is_operator(c2)))
        out += '.';
    }
  }
  return out;
}

} // namespace regexnfa
