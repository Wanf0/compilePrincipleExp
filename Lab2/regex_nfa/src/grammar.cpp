#include "../include/grammar.h"
#include <iostream>
#include <sstream>

static bool isNonterminal(const std::string &s) {
  return !s.empty() && std::isupper(s[0]);
}

// Trim spaces
static std::string trim(const std::string &s) {
  size_t l = s.find_first_not_of(" \t\r\n");
  size_t r = s.find_last_not_of(" \t\r\n");
  if (l == std::string::npos)
    return "";
  return s.substr(l, r - l + 1);
}

// Split by spaces
static std::vector<std::string> split(const std::string &s) {
  std::stringstream ss(s);
  std::string tok;
  std::vector<std::string> v;
  while (ss >> tok)
    v.push_back(tok);
  return v;
}

// Split RHS by "|"
static std::vector<std::string> splitOr(const std::string &rhs) {
  std::vector<std::string> v;
  std::stringstream ss(rhs);
  std::string item;
  while (std::getline(ss, item, '|')) {
    v.push_back(trim(item));
  }
  return v;
}

Grammar Grammar::fromLines(const std::vector<std::string> &lines) {
  Grammar G;

  for (auto &line : lines) {
    if (line.empty())
      continue;

    size_t pos = line.find("->");
    if (pos == std::string::npos)
      continue;

    Symbol left = trim(line.substr(0, pos));
    std::string rhs = trim(line.substr(pos + 2));

    if (G.rules.count(left) == 0) {
      G.nonterminals.push_back(left);
    }

    for (auto &rhsProd : splitOr(rhs)) {
      auto tokens = split(rhsProd);
      if (tokens.empty())
        tokens.push_back("epsilon");
      G.rules[left].push_back(tokens);
    }
  }
  return G;
}

void Grammar::print() const {
  for (auto &A : nonterminals) {
    std::cout << A << " -> ";
    const auto &prods = rules.at(A);
    for (size_t i = 0; i < prods.size(); ++i) {
      for (auto &s : prods[i])
        std::cout << s << " ";
      if (i + 1 != prods.size())
        std::cout << "| ";
    }
    std::cout << "\n";
  }
}

// Substitute Aj productions into Ai:
// For every Ai -> Aj α, replace it with Aj's productions
void Grammar::substitute(const Symbol &Ai, const Symbol &Aj) {
  std::vector<Production> newProds;

  for (auto &prod : rules[Ai]) {
    if (!prod.empty() && prod[0] == Aj) {
      // replace with Aj's productions
      for (auto &pj : rules[Aj]) {
        Production expanded = pj;
        expanded.insert(expanded.end(), prod.begin() + 1, prod.end());
        newProds.push_back(expanded);
      }
    } else {
      newProds.push_back(prod);
    }
  }
  rules[Ai] = newProds;
}

// Handle direct left recursion: A -> A α | β
void Grammar::eliminateDirect(const Symbol &A) {
  std::vector<Production> alphas, betas;

  for (auto &prod : rules[A]) {
    if (!prod.empty() && prod[0] == A) {
      // A -> A α
      Production alpha(prod.begin() + 1, prod.end());
      alphas.push_back(alpha);
    } else {
      betas.push_back(prod);
    }
  }

  if (alphas.empty())
    return; // no direct left recursion

  // Create new A'
  Symbol Aprime = A + "'";
  nonterminals.push_back(Aprime);
  rules[Aprime] = {};

  // A -> β A'
  std::vector<Production> newA;
  for (auto &beta : betas) {
    beta.push_back(Aprime);
    newA.push_back(beta);
  }
  rules[A] = newA;

  // A' -> α A' | epsilon
  std::vector<Production> newAprime;
  for (auto &alpha : alphas) {
    alpha.push_back(Aprime);
    newAprime.push_back(alpha);
  }
  newAprime.push_back({"epsilon"});

  rules[Aprime] = newAprime;
}

void Grammar::eliminateLeftRecursion() {
  int n = nonterminals.size();

  for (int i = 0; i < n; ++i) {
    Symbol Ai = nonterminals[i];

    // Substitute A_j into A_i for j < i
    for (int j = 0; j < i; ++j) {
      Symbol Aj = nonterminals[j];
      substitute(Ai, Aj);
    }

    // Eliminate direct left recursion on Ai
    eliminateDirect(Ai);
  }
}
