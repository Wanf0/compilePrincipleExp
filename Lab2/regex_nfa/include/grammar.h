#ifndef GRAMMAR_H
#define GRAMMAR_H

#include <set>
#include <string>
#include <unordered_map>
#include <vector>

class Grammar {
public:
  using Symbol = std::string;
  using Production = std::vector<Symbol>;

  // Map: Nonterminal -> list of productions
  std::unordered_map<Symbol, std::vector<Production>> rules;

  // Order of nonterminals
  std::vector<Symbol> nonterminals;

public:
  Grammar() = default;

  // Parse from lines such as:   E -> E + T | T
  static Grammar fromLines(const std::vector<std::string> &lines);

  // Print grammar in standard form
  void print() const;

  // Remove indirect and direct left recursion
  void eliminateLeftRecursion();

private:
  void substitute(const Symbol &Ai, const Symbol &Aj);
  void eliminateDirect(const Symbol &Ai);
};

#endif
