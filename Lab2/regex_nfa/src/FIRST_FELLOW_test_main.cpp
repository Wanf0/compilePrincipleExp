#include "../include/first_follow.h"
#include <cassert>
#include <iostream>

void testBasicExample() {
  std::cout << "=== 测试基础示例 ===" << std::endl;

  FirstFollow calculator;
  std::string grammarStr = "S -> A B\n"
                           "A -> a | ε\n"
                           "B -> b";

  calculator.readGrammarFromString(grammarStr);
  calculator.setStartSymbol("S");

  std::cout << "文法:" << std::endl;
  calculator.printGrammar();
  std::cout << std::endl;

  calculator.computeFirstSets();
  calculator.computeFollowSets();

  calculator.printFirstSets();
  std::cout << std::endl;
  calculator.printFollowSets();

  std::cout << std::endl;
}

void testComplexExample() {
  std::cout << "=== 测试复杂示例 ===" << std::endl;

  FirstFollow calculator;
  std::string grammarStr = "E -> T E'\n"
                           "E' -> + T E' | ε\n"
                           "T -> F T'\n"
                           "T' -> * F T' | ε\n"
                           "F -> ( E ) | id";

  calculator.readGrammarFromString(grammarStr);
  calculator.setStartSymbol("E");

  std::cout << "文法:" << std::endl;
  calculator.printGrammar();
  std::cout << std::endl;

  calculator.computeFirstSets();
  calculator.computeFollowSets();

  calculator.printFirstSets();
  std::cout << std::endl;
  calculator.printFollowSets();

  std::cout << std::endl;
}

void testNullableExample() {
  std::cout << "=== 测试可空非终结符 ===" << std::endl;

  FirstFollow calculator;
  std::string grammarStr = "S -> A B C\n"
                           "A -> a | ε\n"
                           "B -> b | ε\n"
                           "C -> c";

  calculator.readGrammarFromString(grammarStr);
  calculator.setStartSymbol("S");

  std::cout << "文法:" << std::endl;
  calculator.printGrammar();
  std::cout << std::endl;

  calculator.computeFirstSets();
  calculator.computeFollowSets();

  calculator.printFirstSets();
  std::cout << std::endl;
  calculator.printFollowSets();

  std::cout << std::endl;
}

void testLeftRecursiveExample() {
  std::cout << "=== 测试左递归文法 ===" << std::endl;

  FirstFollow calculator;
  std::string grammarStr = "Expr -> Expr + Term | Term\n"
                           "Term -> Term * Factor | Factor\n"
                           "Factor -> ( Expr ) | id";

  calculator.readGrammarFromString(grammarStr);
  calculator.setStartSymbol("Expr");

  std::cout << "文法:" << std::endl;
  calculator.printGrammar();
  std::cout << std::endl;

  calculator.computeFirstSets();
  calculator.computeFollowSets();

  calculator.printFirstSets();
  std::cout << std::endl;
  calculator.printFollowSets();

  std::cout << std::endl;
}

void testRightRecursiveExample() {
  std::cout << "=== 测试右递归文法 ===" << std::endl;

  FirstFollow calculator;
  std::string grammarStr = "S -> a S | b";

  calculator.readGrammarFromString(grammarStr);
  calculator.setStartSymbol("S");

  std::cout << "文法:" << std::endl;
  calculator.printGrammar();
  std::cout << std::endl;

  calculator.computeFirstSets();
  calculator.computeFollowSets();

  calculator.printFirstSets();
  std::cout << std::endl;
  calculator.printFollowSets();

  std::cout << std::endl;
}

int main() {
  std::cout << "FIRST和FOLLOW集计算算法测试程序" << std::endl;
  std::cout << "=============================" << std::endl << std::endl;

  try {
    testBasicExample();
    testComplexExample();
    testNullableExample();
    testLeftRecursiveExample();
    testRightRecursiveExample();

    std::cout << "所有测试完成!" << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "测试过程中发生错误: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
