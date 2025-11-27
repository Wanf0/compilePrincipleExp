#ifndef LL1_PARSER_H
#define LL1_PARSER_H

#include "first_follow.h"
#include <map>
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class LL1Parser {
public:
  using Symbol = std::string;
  using Production = std::vector<Symbol>;
  using Productions = std::vector<Production>;
  using Grammar = std::unordered_map<Symbol, Productions>;
  using SymbolSet = std::unordered_set<Symbol>;
  using ParseTable =
      std::unordered_map<Symbol, std::unordered_map<Symbol, Production>>;

  struct ParseResult {
    bool success;
    std::vector<std::string> steps;
    std::string errorMessage;
  };

  LL1Parser();
  ~LL1Parser();

  // 读取文法
  void readGrammar(const Grammar &inputGrammar);
  void readGrammarFromString(const std::string &grammarStr);

  // 设置开始符号
  void setStartSymbol(const Symbol &start);

  // LL(1)判定和预测分析表构建
  bool isLL1Grammar();
  void buildParseTable();

  // 语法分析
  ParseResult parse(const std::string &input);

  // 获取结果
  ParseTable getParseTable() const;
  std::string getLL1Status() const;

  // 打印结果
  void printParseTable() const;
  void printGrammar() const;

  // 清空数据
  void clear();

private:
  Grammar grammar;
  Symbol startSymbol;
  ParseTable parseTable;
  std::unique_ptr<FirstFollow> firstFollowCalculator;
  bool isLL1;
  std::string ll1Status;

  // 工具函数
  bool isTerminal(const Symbol &symbol) const;
  SymbolSet getTerminals() const;
  std::vector<Symbol> getNonTerminals() const; // 改为返回vector
  bool hasIntersection(const SymbolSet &set1, const SymbolSet &set2) const;

  // 新增方法声明
  SymbolSet computeProductionFirst(const Production &production,
                                   const FirstFollow::FirstSet &firstSets);

  // 文法解析
  std::vector<Symbol> splitProduction(const std::string &productionStr);
  std::vector<Symbol> tokenizeInput(const std::string &input);
  std::string trim(const std::string &str);
};

#endif // LL1_PARSER_H
