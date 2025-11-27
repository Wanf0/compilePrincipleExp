#include "../include/ll1_parser.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <stack>
#include <vector>

const std::string EPSILON = "ε";
const std::string END_MARKER = "$";

LL1Parser::LL1Parser() {
  startSymbol = "S";
  isLL1 = false;
  ll1Status = "Not checked";
  firstFollowCalculator = std::make_unique<FirstFollow>();
}

LL1Parser::~LL1Parser() = default;

void LL1Parser::readGrammar(const Grammar &inputGrammar) {
  grammar = inputGrammar;
  if (!grammar.empty()) {
    startSymbol = grammar.begin()->first;
  }
  firstFollowCalculator->readGrammar(grammar);
  firstFollowCalculator->setStartSymbol(startSymbol);
  isLL1 = false;
  ll1Status = "Grammar updated, not checked";
}

void LL1Parser::readGrammarFromString(const std::string &grammarStr) {
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

  firstFollowCalculator->readGrammar(grammar);
  firstFollowCalculator->setStartSymbol(startSymbol);
  isLL1 = false;
  ll1Status = "Grammar updated, not checked";
}

void LL1Parser::setStartSymbol(const Symbol &start) {
  startSymbol = start;
  firstFollowCalculator->setStartSymbol(start);
}

bool LL1Parser::isLL1Grammar() {
  if (grammar.empty()) {
    ll1Status = "Empty grammar";
    return false;
  }

  // 计算FIRST和FOLLOW集
  firstFollowCalculator->computeFirstSets();
  firstFollowCalculator->computeFollowSets();

  auto firstSets = firstFollowCalculator->getFirstSets();
  auto followSets = firstFollowCalculator->getFollowSets();

  // 检查每个非终结符的LL(1)条件
  for (const auto &[nonterminal, productions] : grammar) {
    // 对于每对不同的产生式
    for (size_t i = 0; i < productions.size(); ++i) {
      for (size_t j = i + 1; j < productions.size(); ++j) {
        const auto &alpha = productions[i];
        const auto &beta = productions[j];

        // 计算FIRST(alpha)和FIRST(beta)
        SymbolSet firstAlpha = computeProductionFirst(alpha, firstSets);
        SymbolSet firstBeta = computeProductionFirst(beta, firstSets);

        // 检查条件1: FIRST(alpha) ∩ FIRST(beta) = ∅
        if (hasIntersection(firstAlpha, firstBeta)) {
          ll1Status = "FIRST set conflict for " + nonterminal;
          isLL1 = false;
          return false;
        }

        // 检查条件2和3: 如果ε在FIRST(alpha)中，则FIRST(beta) ∩ FOLLOW(A) = ∅
        if (firstAlpha.find(EPSILON) != firstAlpha.end()) {
          const auto &followA = followSets.at(nonterminal);
          if (hasIntersection(firstBeta, followA)) {
            ll1Status = "FOLLOW set conflict for " + nonterminal;
            isLL1 = false;
            return false;
          }
        }

        // 检查条件3: 如果ε在FIRST(beta)中，则FIRST(alpha) ∩ FOLLOW(A) = ∅
        if (firstBeta.find(EPSILON) != firstBeta.end()) {
          const auto &followA = followSets.at(nonterminal);
          if (hasIntersection(firstAlpha, followA)) {
            ll1Status = "FOLLOW set conflict for " + nonterminal;
            isLL1 = false;
            return false;
          }
        }
      }
    }
  }

  isLL1 = true;
  ll1Status = "This is an LL(1) grammar";
  return true;
}

LL1Parser::SymbolSet
LL1Parser::computeProductionFirst(const Production &production,
                                  const FirstFollow::FirstSet &firstSets) {
  SymbolSet result;

  if (production.empty()) {
    result.insert(EPSILON);
    return result;
  }

  bool allNullable = true;

  for (const auto &symbol : production) {
    if (isTerminal(symbol)) {
      result.insert(symbol);
      allNullable = false;
      break;
    } else {
      // 非终结符
      const auto &first = firstSets.at(symbol);
      bool symbolNullable = false;

      for (const auto &s : first) {
        if (s == EPSILON) {
          symbolNullable = true;
        } else {
          result.insert(s);
        }
      }

      if (!symbolNullable) {
        allNullable = false;
        break;
      }
    }
  }

  if (allNullable) {
    result.insert(EPSILON);
  }

  return result;
}

void LL1Parser::buildParseTable() {
  if (!isLL1) {
    if (!isLL1Grammar()) {
      return;
    }
  }

  parseTable.clear();

  auto firstSets = firstFollowCalculator->getFirstSets();
  auto followSets = firstFollowCalculator->getFollowSets();

  // 获取所有终结符（包括$）
  SymbolSet terminals = getTerminals();
  terminals.insert(END_MARKER);

  // 填充分析表
  for (const auto &[nonterminal, productions] : grammar) {
    for (const auto &production : productions) {
      SymbolSet firstProd = computeProductionFirst(production, firstSets);

      // 对于FIRST(production)中的每个终结符a
      for (const auto &terminal : firstProd) {
        if (terminal != EPSILON) {
          // 检查是否已经存在产生式（冲突）
          if (parseTable[nonterminal].find(terminal) !=
              parseTable[nonterminal].end()) {
            std::cerr << "Warning: Parse table conflict for [" << nonterminal
                      << ", " << terminal << "]" << std::endl;
          }
          parseTable[nonterminal][terminal] = production;
        }
      }

      // 如果ε在FIRST(production)中
      if (firstProd.find(EPSILON) != firstProd.end()) {
        const auto &followA = followSets.at(nonterminal);
        for (const auto &terminal : followA) {
          // 检查是否已经存在产生式（冲突）
          if (parseTable[nonterminal].find(terminal) !=
              parseTable[nonterminal].end()) {
            std::cerr << "Warning: Parse table conflict for [" << nonterminal
                      << ", " << terminal << "]" << std::endl;
          }
          parseTable[nonterminal][terminal] = production;
        }
        // 同时处理$符号
        if (followA.find(END_MARKER) != followA.end()) {
          parseTable[nonterminal][END_MARKER] = production;
        }
      }
    }
  }
}

LL1Parser::ParseResult LL1Parser::parse(const std::string &input) {
  ParseResult result;
  result.success = false;

  if (!isLL1) {
    result.errorMessage = "Grammar is not LL(1)";
    return result;
  }

  if (parseTable.empty()) {
    buildParseTable();
  }

  // 初始化栈和输入
  std::stack<Symbol> stack;
  stack.push(END_MARKER);
  stack.push(startSymbol);

  std::vector<Symbol> inputTokens = tokenizeInput(input);
  inputTokens.push_back(END_MARKER);

  size_t inputPos = 0;

  // 记录分析步骤
  std::string initialStack = startSymbol + std::string(1, END_MARKER[0]);
  std::string initialInput = input + (input.empty() ? "" : " ") + END_MARKER;
  result.steps.push_back("Initial: Stack: " + initialStack +
                         ", Input: " + initialInput);

  while (!stack.empty()) {
    Symbol top = stack.top();
    Symbol currentInput = inputTokens[inputPos];

    // 记录当前状态
    std::string stackStr;
    std::stack<Symbol> tempStack = stack;
    std::vector<Symbol> stackSymbols;
    while (!tempStack.empty()) {
      stackSymbols.push_back(tempStack.top());
      tempStack.pop();
    }
    for (auto it = stackSymbols.rbegin(); it != stackSymbols.rend(); ++it) {
      stackStr += *it;
    }

    std::string inputStr;
    for (size_t i = inputPos; i < inputTokens.size(); ++i) {
      inputStr += inputTokens[i] + " ";
    }
    if (!inputStr.empty())
      inputStr.pop_back(); // 移除末尾空格

    std::string step = "Stack: " + stackStr + ", Input: " + inputStr;

    if (top == END_MARKER) {
      if (currentInput == END_MARKER) {
        result.steps.push_back(step + " -> ACCEPT");
        result.success = true;
        break;
      } else {
        result.steps.push_back(step +
                               " -> ERROR: Stack empty but input remains");
        result.errorMessage = "Stack empty but input remains";
        break;
      }
    }

    if (isTerminal(top)) {
      if (top == currentInput) {
        stack.pop();
        inputPos++;
        result.steps.push_back(step + " -> Match: " + top);
      } else {
        result.steps.push_back(step + " -> ERROR: Terminal mismatch");
        result.errorMessage =
            "Terminal mismatch: expected " + top + ", got " + currentInput;
        break;
      }
    } else {
      // 非终结符，查分析表
      auto ntIt = parseTable.find(top);
      if (ntIt != parseTable.end()) {
        const auto &row = ntIt->second;
        auto termIt = row.find(currentInput);

        if (termIt != row.end()) {
          const Production &production = termIt->second;
          stack.pop();

          // 将产生式右部逆序压栈（ε产生式不压入任何内容）
          if (!production.empty()) {
            for (auto it = production.rbegin(); it != production.rend(); ++it) {
              stack.push(*it);
            }
          }

          // 记录使用的产生式
          std::string prodStr = top + " -> ";
          if (production.empty()) {
            prodStr += EPSILON;
          } else {
            for (const auto &sym : production) {
              prodStr += sym + " ";
            }
          }
          result.steps.push_back(step + " -> Apply: " + prodStr);
        } else {
          result.steps.push_back(step +
                                 " -> ERROR: No production in parse table");
          result.errorMessage =
              "No production for " + top + " on input " + currentInput;
          break;
        }
      } else {
        result.steps.push_back(step +
                               " -> ERROR: Nonterminal not in parse table");
        result.errorMessage = "Nonterminal " + top + " not in parse table";
        break;
      }
    }
  }

  return result;
}

LL1Parser::ParseTable LL1Parser::getParseTable() const { return parseTable; }

std::string LL1Parser::getLL1Status() const { return ll1Status; }

void LL1Parser::printParseTable() const {
  if (parseTable.empty()) {
    std::cout << "Parse table not built yet." << std::endl;
    return;
  }

  // 收集所有终结符
  SymbolSet terminals = getTerminals();
  terminals.insert(END_MARKER);

  // 将终结符排序以便更好的显示
  std::vector<Symbol> sortedTerminals(terminals.begin(), terminals.end());
  std::sort(sortedTerminals.begin(), sortedTerminals.end());

  std::cout << "LL(1) Parse Table:" << std::endl;
  std::cout << "NonTerminal\\Terminal";
  for (const auto &terminal : sortedTerminals) {
    std::cout << "\t" << terminal;
  }
  std::cout << std::endl;

  // 将非终结符排序
  std::vector<Symbol> sortedNonTerminals = getNonTerminals();
  std::sort(sortedNonTerminals.begin(), sortedNonTerminals.end());

  for (const auto &nonterminal : sortedNonTerminals) {
    std::cout << nonterminal;
    const auto &row = parseTable.at(nonterminal);
    for (const auto &terminal : sortedTerminals) {
      std::cout << "\t";
      auto it = row.find(terminal);
      if (it != row.end()) {
        const Production &production = it->second;
        if (production.empty()) {
          std::cout << EPSILON;
        } else {
          for (const auto &sym : production) {
            std::cout << sym;
          }
        }
      } else {
        std::cout << " ";
      }
    }
    std::cout << std::endl;
  }
}

void LL1Parser::printGrammar() const {
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

void LL1Parser::clear() {
  grammar.clear();
  parseTable.clear();
  isLL1 = false;
  ll1Status = "Cleared";
  firstFollowCalculator->clear();
}

bool LL1Parser::isTerminal(const Symbol &symbol) const {
  if (symbol == EPSILON || symbol == END_MARKER)
    return false;
  return grammar.find(symbol) == grammar.end();
}

LL1Parser::SymbolSet LL1Parser::getTerminals() const {
  SymbolSet terminals;

  for (const auto &[nt, prods] : grammar) {
    for (const auto &prod : prods) {
      for (const auto &symbol : prod) {
        if (isTerminal(symbol)) {
          terminals.insert(symbol);
        }
      }
    }
  }

  return terminals;
}

std::vector<LL1Parser::Symbol> LL1Parser::getNonTerminals() const {
  std::vector<Symbol> nonTerminals;
  for (const auto &[nt, _] : grammar) {
    nonTerminals.push_back(nt);
  }
  return nonTerminals;
}

bool LL1Parser::hasIntersection(const SymbolSet &set1,
                                const SymbolSet &set2) const {
  for (const auto &symbol : set1) {
    if (set2.find(symbol) != set2.end()) {
      return true;
    }
  }
  return false;
}

std::vector<LL1Parser::Symbol>
LL1Parser::splitProduction(const std::string &productionStr) {
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

std::vector<LL1Parser::Symbol>
LL1Parser::tokenizeInput(const std::string &input) {
  std::vector<Symbol> tokens;
  std::istringstream iss(input);
  std::string token;

  while (iss >> token) {
    token = trim(token);
    if (!token.empty()) {
      tokens.push_back(token);
    }
  }

  return tokens;
}

std::string LL1Parser::trim(const std::string &str) {
  size_t start = str.find_first_not_of(" \t\n\r");
  if (start == std::string::npos)
    return "";

  size_t end = str.find_last_not_of(" \t\n\r");
  return str.substr(start, end - start + 1);
}
