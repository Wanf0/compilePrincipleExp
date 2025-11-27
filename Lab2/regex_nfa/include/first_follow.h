#ifndef FIRST_FOLLOW_H
#define FIRST_FOLLOW_H

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class FirstFollow {
public:
  using Symbol = std::string;
  using Production = std::vector<Symbol>;
  using Productions = std::vector<Production>;
  using Grammar = std::unordered_map<Symbol, Productions>;

  using SymbolSet = std::unordered_set<Symbol>;
  using FirstSet = std::unordered_map<Symbol, SymbolSet>;
  using FollowSet = std::unordered_map<Symbol, SymbolSet>;

  FirstFollow();
  ~FirstFollow();

  void readGrammar(const Grammar &inputGrammar);
  void readGrammarFromString(const std::string &grammarStr);

  void setStartSymbol(const Symbol &start);

  void computeFirstSets();
  void computeFollowSets();

  FirstSet getFirstSets() const;
  FollowSet getFollowSets() const;

  void printFirstSets() const;
  void printFollowSets() const;
  void printGrammar() const;

  void clear();

private:
  Grammar grammar;
  Symbol startSymbol;
  FirstSet firstSets;
  FollowSet followSets;

  // 修复：添加nullable计算
  std::unordered_map<Symbol, bool> nullable;

  bool isTerminal(const Symbol &symbol) const;
  void computeNullable();
  bool computeProductionFirst(const Production &production, SymbolSet &result);

  std::vector<Symbol> splitProduction(const std::string &productionStr);
  std::string trim(const std::string &str);
};

#endif // FIRST_FOLLOW_H
