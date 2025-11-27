#include "../include/left_factor.h"
#include <cassert>
#include <iostream>

void testBasicExample() {
  std::cout << "=== 测试基础示例 ===" << std::endl;

  LeftFactor factorer;
  std::string grammarStr =
      "S -> apple | apply | application | ball | bat | bath | Xb\n"
      "X -> ab | ac | ad";

  factorer.readGrammarFromString(grammarStr);
  std::cout << "原始文法:" << std::endl;
  factorer.printOriginal();

  factorer.leftFactorGrammar();
  std::cout << "\n提取左公共因子后的文法:" << std::endl;
  factorer.printResult();

  std::cout << std::endl;
}

void testNestedFactorization() {
  std::cout << "=== 测试嵌套左因子提取 ===" << std::endl;

  LeftFactor factorer;
  std::string grammarStr = "A -> abc | abx | aby | abcd | abce";

  factorer.readGrammarFromString(grammarStr);
  std::cout << "原始文法:" << std::endl;
  factorer.printOriginal();

  factorer.leftFactorGrammar();
  std::cout << "\n提取左公共因子后的文法:" << std::endl;
  factorer.printResult();

  std::cout << std::endl;
}

void testCompletePrefix() {
  std::cout << "=== 测试完全前缀情况 ===" << std::endl;

  LeftFactor factorer;
  std::string grammarStr = "B -> hello | helloX | helloY";

  factorer.readGrammarFromString(grammarStr);
  std::cout << "原始文法:" << std::endl;
  factorer.printOriginal();

  factorer.leftFactorGrammar();
  std::cout << "\n提取左公共因子后的文法:" << std::endl;
  factorer.printResult();

  std::cout << std::endl;
}

void testNoCommonPrefix() {
  std::cout << "=== 测试无公共前缀情况 ===" << std::endl;

  LeftFactor factorer;
  std::string grammarStr = "C -> apple | banana | cherry";

  factorer.readGrammarFromString(grammarStr);
  std::cout << "原始文法:" << std::endl;
  factorer.printOriginal();

  factorer.leftFactorGrammar();
  std::cout << "\n提取左公共因子后的文法:" << std::endl;
  factorer.printResult();

  std::cout << std::endl;
}

void testMultipleNonterminals() {
  std::cout << "=== 测试多非终结符 ===" << std::endl;

  LeftFactor factorer;
  std::string grammarStr =
      "S -> if expr then stmt | if expr then stmt else stmt\n"
      "Expr -> id | num | Expr + Expr | Expr * Expr";

  factorer.readGrammarFromString(grammarStr);
  std::cout << "原始文法:" << std::endl;
  factorer.printOriginal();

  factorer.leftFactorGrammar();
  std::cout << "\n提取左公共因子后的文法:" << std::endl;
  factorer.printResult();

  std::cout << std::endl;
}

void testProgrammaticInput() {
  std::cout << "=== 测试编程方式输入 ===" << std::endl;

  LeftFactor factorer;
  LeftFactor::Grammar grammar = {{"S", {"axy", "axz", "bxy", "bxz", "c"}},
                                 {"X", {"pq", "pr", "ps"}}};

  factorer.readGrammar(grammar);
  std::cout << "原始文法:" << std::endl;
  factorer.printOriginal();

  factorer.leftFactorGrammar();
  std::cout << "\n提取左公共因子后的文法:" << std::endl;
  factorer.printResult();

  std::cout << std::endl;
}

int main() {
  std::cout << "左公共因子提取算法测试程序" << std::endl;
  std::cout << "=========================" << std::endl << std::endl;

  try {
    testBasicExample();
    testNestedFactorization();
    testCompletePrefix();
    testNoCommonPrefix();
    testMultipleNonterminals();
    testProgrammaticInput();

    std::cout << "所有测试完成!" << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "测试过程中发生错误: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
