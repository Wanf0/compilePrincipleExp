#include "../include/first_follow.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <stack>

const std::string EPSILON = "ε";
const std::string END_MARKER = "$";

FirstFollow::FirstFollow() { startSymbol = "S"; }

FirstFollow::~FirstFollow() = default;

void FirstFollow::readGrammar(const Grammar &inputGrammar) {
  grammar = inputGrammar;
  if (!grammar.empty()) {
    startSymbol = grammar.begin()->first;
  }
}

void FirstFollow::readGrammarFromString(const std::string &grammarStr) {
  grammar.clear();

  std::istringstream iss(grammarStr);
  std::string line;

  while (std::getline(iss, line)) {
    line = trim(line);
    if (line.empty())
      continue;

    size_t arrowPos = line.find("->");
    if (arrowPos == std::string::npos) {
      continue;
    }

    std::string nonterminal = trim(line.substr(0, arrowPos));
    std::string productionsStr = trim(line.substr(arrowPos + 2));

    std::istringstream prodIss(productionsStr);
    std::string prodStr;
    Productions productions;

    while (std::getline(prodIss, prodStr, '|')) {
      prodStr = trim(prodStr);
      if (!prodStr.empty()) {
        if (prodStr == EPSILON) {
          // ε产生式表示为空向量
          productions.push_back({});
        } else {
          productions.push_back(splitProduction(prodStr));
        }
      }
    }

    grammar[nonterminal] = productions;
  }

  if (!grammar.empty() && startSymbol.empty()) {
    startSymbol = grammar.begin()->first;
  }
}

void FirstFollow::setStartSymbol(const Symbol &start) { startSymbol = start; }

void FirstFollow::computeFirstSets() {
  firstSets.clear();
  computeNullable();

  // 初始化：终结符的FIRST集就是自身
  for (const auto &[nt, prods] : grammar) {
    for (const auto &prod : prods) {
      for (const auto &symbol : prod) {
        if (isTerminal(symbol)) {
          firstSets[symbol].insert(symbol);
        }
      }
    }
  }

  // 修复：迭代计算非终结符的FIRST集
  bool changed;
  do {
    changed = false;

    for (const auto &[nonterminal, productions] : grammar) {
      SymbolSet &firstSet = firstSets[nonterminal];
      size_t oldSize = firstSet.size();

      for (const auto &production : productions) {
        SymbolSet prodFirst;
        if (computeProductionFirst(production, prodFirst)) {
          // 整个产生式可推出ε
          prodFirst.insert(EPSILON);
        }

        // 合并到当前非终结符的FIRST集
        for (const auto &symbol : prodFirst) {
          if (symbol != EPSILON || prodFirst.size() == 1) {
            firstSet.insert(symbol);
          }
        }
      }

      if (firstSet.size() != oldSize) {
        changed = true;
      }
    }
  } while (changed);
}

void FirstFollow::computeFollowSets() {
  followSets.clear();

  // 初始化：开始符号的FOLLOW集包含$
  followSets[startSymbol].insert(END_MARKER);

  // 修复：迭代计算FOLLOW集
  bool changed;
  do {
    changed = false;

    for (const auto &[nonterminal, productions] : grammar) {
      for (const auto &production : productions) {
        // 遍历产生式中的每个符号
        for (size_t i = 0; i < production.size(); ++i) {
          const Symbol &current = production[i];

          if (isTerminal(current)) {
            continue;
          }

          SymbolSet &followSet = followSets[current];
          size_t oldSize = followSet.size();

          // 情况1: A -> αBβ，将FIRST(β)-{ε}加入FOLLOW(B)
          if (i + 1 < production.size()) {
            Production beta(production.begin() + i + 1, production.end());
            SymbolSet firstBeta;
            bool betaNullable = computeProductionFirst(beta, firstBeta);

            for (const auto &symbol : firstBeta) {
              if (symbol != EPSILON) {
                followSet.insert(symbol);
              }
            }

            // 情况2: 如果β可推出ε，将FOLLOW(A)加入FOLLOW(B)
            if (betaNullable) {
              const SymbolSet &followA = followSets[nonterminal];
              for (const auto &symbol : followA) {
                followSet.insert(symbol);
              }
            }
          } else {
            // 情况3: A -> αB，将FOLLOW(A)加入FOLLOW(B)
            const SymbolSet &followA = followSets[nonterminal];
            for (const auto &symbol : followA) {
              followSet.insert(symbol);
            }
          }

          if (followSet.size() != oldSize) {
            changed = true;
          }
        }
      }
    }
  } while (changed);
}

void FirstFollow::computeNullable() {
  nullable.clear();

  // 初始化
  for (const auto &[nt, _] : grammar) {
    nullable[nt] = false;
  }

  // 修复：计算可空非终结符
  bool changed;
  do {
    changed = false;

    for (const auto &[nonterminal, productions] : grammar) {
      if (nullable[nonterminal]) {
        continue; // 已经可空
      }

      for (const auto &production : productions) {
        if (production.empty()) {
          // 空产生式
          nullable[nonterminal] = true;
          changed = true;
          break;
        }

        bool allNullable = true;
        for (const auto &symbol : production) {
          if (isTerminal(symbol) || !nullable[symbol]) {
            allNullable = false;
            break;
          }
        }

        if (allNullable) {
          nullable[nonterminal] = true;
          changed = true;
          break;
        }
      }
    }
  } while (changed);
}

bool FirstFollow::computeProductionFirst(const Production &production,
                                         SymbolSet &result) {
  if (production.empty()) {
    return true; // 空产生式可推出ε
  }

  bool allNullable = true;

  for (const auto &symbol : production) {
    if (isTerminal(symbol)) {
      result.insert(symbol);
      allNullable = false;
      break;
    } else {
      // 非终结符
      const SymbolSet &first = firstSets[symbol];
      bool symbolNullable = nullable[symbol];

      // 添加FIRST(symbol)中除了ε的所有符号
      for (const auto &s : first) {
        if (s != EPSILON) {
          result.insert(s);
        }
      }

      if (!symbolNullable) {
        allNullable = false;
        break;
      }
    }
  }

  return allNullable;
}

bool FirstFollow::isTerminal(const Symbol &symbol) const {
  if (symbol == EPSILON)
    return false;
  return grammar.find(symbol) == grammar.end();
}

FirstFollow::FirstSet FirstFollow::getFirstSets() const { return firstSets; }

FirstFollow::FollowSet FirstFollow::getFollowSets() const { return followSets; }

void FirstFollow::printFirstSets() const {
  std::cout << "FIRST Sets:" << std::endl;

  // 只显示非终结符的FIRST集
  for (const auto &[symbol, firstSet] : firstSets) {
    if (!isTerminal(symbol)) {
      std::cout << "FIRST(" << symbol << ") = { ";
      bool first = true;
      for (const auto &s : firstSet) {
        if (!first)
          std::cout << ", ";
        std::cout << s;
        first = false;
      }
      std::cout << " }" << std::endl;
    }
  }
}

void FirstFollow::printFollowSets() const {
  std::cout << "FOLLOW Sets:" << std::endl;
  for (const auto &[symbol, followSet] : followSets) {
    std::cout << "FOLLOW(" << symbol << ") = { ";
    bool first = true;
    for (const auto &s : followSet) {
      if (!first)
        std::cout << ", ";
      std::cout << s;
      first = false;
    }
    std::cout << " }" << std::endl;
  }
}

void FirstFollow::printGrammar() const {
  std::cout << "Grammar:" << std::endl;
  for (const auto &[nonterminal, productions] : grammar) {
    std::cout << nonterminal << " -> ";
    for (size_t i = 0; i < productions.size(); ++i) {
      if (productions[i].empty()) {
        std::cout << EPSILON;
      } else {
        for (const auto &symbol : productions[i]) {
          std::cout << symbol << " ";
        }
      }
      if (i < productions.size() - 1) {
        std::cout << "| ";
      }
    }
    std::cout << std::endl;
  }
}

void FirstFollow::clear() {
  grammar.clear();
  firstSets.clear();
  followSets.clear();
  nullable.clear();
  startSymbol = "S";
}

std::vector<FirstFollow::Symbol>
FirstFollow::splitProduction(const std::string &productionStr) {
  std::vector<Symbol> symbols;
  std::istringstream iss(productionStr);
  std::string symbol;

  while (iss >> symbol) {
    symbol = trim(symbol);
    if (!symbol.empty()) {
      symbols.push_back(symbol);
    }
  }

  return symbols;
}

std::string FirstFollow::trim(const std::string &str) {
  size_t start = str.find_first_not_of(" \t\n\r");
  if (start == std::string::npos)
    return "";

  size_t end = str.find_last_not_of(" \t\n\r");
  return str.substr(start, end - start + 1);
}
