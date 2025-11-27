#include "../include/left_factor.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stack>

LeftFactor::LeftFactor() = default;

LeftFactor::~LeftFactor() = default;

void LeftFactor::readGrammar(const Grammar &inputGrammar) {
  originalGrammar = inputGrammar;
  factoredGrammar = inputGrammar;
}

void LeftFactor::readGrammarFromString(const std::string &grammarStr) {
  originalGrammar.clear();
  factoredGrammar.clear();

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

    std::vector<std::string> productions = splitProductions(productionsStr);
    originalGrammar[nonterminal] = productions;
    factoredGrammar[nonterminal] = productions;
  }
}

void LeftFactor::leftFactorGrammar() {
  // 先处理所有原始非终结符
  std::vector<std::string> nonterminals;
  for (const auto &[nt, _] : factoredGrammar) {
    nonterminals.push_back(nt);
  }

  for (const auto &nt : nonterminals) {
    recursiveFactor(nt);
  }
}

void LeftFactor::recursiveFactor(const std::string &nonterminal) {
  auto it = factoredGrammar.find(nonterminal);
  if (it == factoredGrammar.end() || it->second.size() < 2) {
    return;
  }

  std::vector<std::string> &productions = it->second;

  // 查找所有公共前缀
  std::map<std::string, std::vector<std::string>> prefixGroups;

  // 按第一个字符分组
  for (const auto &prod : productions) {
    if (!prod.empty()) {
      std::string firstChar = std::string(1, prod[0]);
      prefixGroups[firstChar].push_back(prod);
    } else {
      // 处理空产生式
      prefixGroups["ε"].push_back(prod);
    }
  }

  // 检查每个分组是否需要进一步分解
  bool changed = false;
  std::vector<std::string> newProductions;

  for (const auto &[firstChar, group] : prefixGroups) {
    if (group.size() == 1) {
      // 没有公共前缀，直接保留
      newProductions.push_back(group[0]);
    } else {
      // 查找最长公共前缀
      std::string lcp = findLongestCommonPrefix(group);

      if (lcp.length() > 0) {
        // 提取公共前缀
        std::string newNT = generateNewNonterminal(nonterminal);
        std::vector<std::string> suffixes;

        for (const auto &prod : group) {
          std::string suffix = prod.substr(lcp.length());
          if (suffix.empty()) {
            suffix = "ε";
          }
          suffixes.push_back(suffix);
        }

        // 添加新产生式
        newProductions.push_back(lcp + newNT);
        factoredGrammar[newNT] = suffixes;

        // 递归处理新非终结符
        recursiveFactor(newNT);

        changed = true;
      } else {
        // 没有公共前缀，保留原产生式
        for (const auto &prod : group) {
          newProductions.push_back(prod);
        }
      }
    }
  }

  // 如果有变化，更新文法
  if (changed) {
    factoredGrammar[nonterminal] = newProductions;
    // 重新处理当前非终结符，可能有新的公共前缀
    recursiveFactor(nonterminal);
  }
}

std::string
LeftFactor::findLongestCommonPrefix(const std::vector<std::string> &strings) {
  if (strings.empty())
    return "";

  std::string prefix = strings[0];

  for (size_t i = 1; i < strings.size(); i++) {
    std::string current = strings[i];
    size_t j = 0;
    while (j < prefix.length() && j < current.length() &&
           prefix[j] == current[j]) {
      j++;
    }
    prefix = prefix.substr(0, j);

    if (prefix.empty()) {
      break;
    }
  }

  return prefix;
}

// 以下Trie相关方法暂时保留但不使用
std::shared_ptr<LeftFactor::TrieNode>
LeftFactor::buildTrie(const std::vector<std::string> &productions) {
  return nullptr;
}

void LeftFactor::findBranches(std::shared_ptr<TrieNode> node,
                              const std::string &currentPrefix,
                              std::vector<BranchInfo> &branches) {
  // 空实现
}

void LeftFactor::collectSuffixes(std::shared_ptr<TrieNode> node,
                                 const std::string &currentSuffix,
                                 std::vector<std::string> &suffixes) {
  // 空实现
}

std::string LeftFactor::generateNewNonterminal(const std::string &base) {
  // 使用数字后缀确保唯一性
  static std::map<std::string, int> counters;
  int count = ++counters[base];
  return base + "_" + std::to_string(count);
}

LeftFactor::Grammar LeftFactor::getResult() const { return factoredGrammar; }

void LeftFactor::printResult() const {
  // 先打印原始非终结符，再打印新生成的
  std::set<std::string> originalNTs;
  for (const auto &[nt, _] : originalGrammar) {
    originalNTs.insert(nt);
  }

  // 打印原始非终结符
  for (const auto &nt : originalNTs) {
    auto it = factoredGrammar.find(nt);
    if (it != factoredGrammar.end()) {
      std::cout << nt << " -> ";
      for (size_t i = 0; i < it->second.size(); ++i) {
        std::cout << it->second[i];
        if (i < it->second.size() - 1) {
          std::cout << " | ";
        }
      }
      std::cout << std::endl;
    }
  }

  // 打印新生成的非终结符
  for (const auto &[nt, prods] : factoredGrammar) {
    if (originalNTs.find(nt) == originalNTs.end()) {
      std::cout << nt << " -> ";
      for (size_t i = 0; i < prods.size(); ++i) {
        std::cout << prods[i];
        if (i < prods.size() - 1) {
          std::cout << " | ";
        }
      }
      std::cout << std::endl;
    }
  }
}

void LeftFactor::printOriginal() const {
  for (const auto &[nonterminal, productions] : originalGrammar) {
    std::cout << nonterminal << " -> ";
    for (size_t i = 0; i < productions.size(); ++i) {
      std::cout << productions[i];
      if (i < productions.size() - 1) {
        std::cout << " | ";
      }
    }
    std::cout << std::endl;
  }
}

void LeftFactor::clear() {
  originalGrammar.clear();
  factoredGrammar.clear();
  nonTerminalCounter = 1;
}

std::vector<std::string>
LeftFactor::splitProductions(const std::string &productionStr) {
  std::vector<std::string> productions;
  std::istringstream iss(productionStr);
  std::string production;

  while (std::getline(iss, production, '|')) {
    production = trim(production);
    if (!production.empty()) {
      productions.push_back(production);
    }
  }

  return productions;
}

std::string LeftFactor::trim(const std::string &str) {
  size_t start = str.find_first_not_of(" \t\n\r");
  if (start == std::string::npos)
    return "";

  size_t end = str.find_last_not_of(" \t\n\r");
  return str.substr(start, end - start + 1);
}
