#ifndef LEFT_FACTOR_H
#define LEFT_FACTOR_H

#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

class LeftFactor {
public:
  using Grammar = std::unordered_map<std::string, std::vector<std::string>>;

  LeftFactor();
  ~LeftFactor();

  void readGrammar(const Grammar &inputGrammar);
  void readGrammarFromString(const std::string &grammarStr);

  void leftFactorGrammar();

  Grammar getResult() const;
  void printResult() const;
  void printOriginal() const;

  void clear();

private:
  struct TrieNode {
    std::unordered_map<char, std::shared_ptr<TrieNode>> children;
    bool isEnd = false;
    int count = 0;

    TrieNode() = default;
  };

  struct BranchInfo {
    std::string prefix;
    std::vector<std::string> suffixes;
  };

private:
  Grammar originalGrammar;
  Grammar factoredGrammar;
  int nonTerminalCounter = 1;
  static const int MAX_RECURSION_DEPTH = 10;

  // 新增方法声明
  void recursiveFactor(const std::string &nonterminal);
  std::string findLongestCommonPrefix(const std::vector<std::string> &strings);

  // 原有的Trie方法（暂时保留，但不再使用）
  std::shared_ptr<TrieNode>
  buildTrie(const std::vector<std::string> &productions);
  void findBranches(std::shared_ptr<TrieNode> node,
                    const std::string &currentPrefix,
                    std::vector<BranchInfo> &branches);
  void collectSuffixes(std::shared_ptr<TrieNode> node,
                       const std::string &currentSuffix,
                       std::vector<std::string> &suffixes);

  std::string generateNewNonterminal(const std::string &base);

  std::vector<std::string> splitProductions(const std::string &productionStr);
  std::string trim(const std::string &str);
};

#endif // LEFT_FACTOR_H
