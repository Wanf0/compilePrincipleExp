#include "../include/ll1_parser.h"
#include <cassert>
#include <iostream>

void testLL1Grammar() {
  std::cout << "=== 测试LL(1)文法 ===" << std::endl;

  LL1Parser parser;
  std::string grammarStr = "E -> T E'\n"
                           "E' -> + T E' | ε\n"
                           "T -> F T'\n"
                           "T' -> * F T' | ε\n"
                           "F -> ( E ) | id";

  parser.readGrammarFromString(grammarStr);
  parser.setStartSymbol("E");

  std::cout << "文法:" << std::endl;
  parser.printGrammar();
  std::cout << std::endl;

  bool isLL1 = parser.isLL1Grammar();
  std::cout << "LL(1)状态: " << parser.getLL1Status() << std::endl;

  if (isLL1) {
    parser.buildParseTable();
    std::cout << std::endl;
    parser.printParseTable();

    // 测试语法分析
    std::cout << std::endl << "=== 语法分析测试 ===" << std::endl;
    std::vector<std::string> testInputs = {"id + id * id", "id * id",
                                           "( id + id ) * id"};

    for (const auto &input : testInputs) {
      std::cout << "分析输入: " << input << std::endl;
      auto result = parser.parse(input);
      if (result.success) {
        std::cout << "分析成功!" << std::endl;
      } else {
        std::cout << "分析失败: " << result.errorMessage << std::endl;
      }
      std::cout << "分析步骤:" << std::endl;
      for (const auto &step : result.steps) {
        std::cout << "  " << step << std::endl;
      }
      std::cout << std::endl;
    }
  }

  std::cout << std::endl;
}

void testNewExample() {
  std::cout << "=== 测试新样例 S → AB, A → aA | ε, B → b ===" << std::endl;

  LL1Parser parser;
  std::string grammarStr = "S -> A B\n"
                           "A -> a A | ε\n"
                           "B -> b";

  parser.readGrammarFromString(grammarStr);
  parser.setStartSymbol("S");

  std::cout << "文法:" << std::endl;
  parser.printGrammar();
  std::cout << std::endl;

  bool isLL1 = parser.isLL1Grammar();
  std::cout << "LL(1)状态: " << parser.getLL1Status() << std::endl;

  if (isLL1) {
    parser.buildParseTable();
    std::cout << std::endl;
    parser.printParseTable();

    // 测试语法分析
    std::cout << std::endl << "=== 语法分析测试 ===" << std::endl;
    std::vector<std::string> testInputs = {"a a b", "a b", "b"};

    for (const auto &input : testInputs) {
      std::cout << "分析输入: " << input << std::endl;
      auto result = parser.parse(input);
      if (result.success) {
        std::cout << "分析成功!" << std::endl;
      } else {
        std::cout << "分析失败: " << result.errorMessage << std::endl;
      }
      std::cout << "分析步骤:" << std::endl;
      for (const auto &step : result.steps) {
        std::cout << "  " << step << std::endl;
      }
      std::cout << std::endl;
    }
  }

  std::cout << std::endl;
}

void testNonLL1Grammar() {
  std::cout << "=== 测试非LL(1)文法 ===" << std::endl;

  LL1Parser parser;
  std::string grammarStr = "S -> A B | A C\n"
                           "A -> a | ε\n"
                           "B -> b\n"
                           "C -> c";

  parser.readGrammarFromString(grammarStr);
  parser.setStartSymbol("S");

  std::cout << "文法:" << std::endl;
  parser.printGrammar();
  std::cout << std::endl;

  bool isLL1 = parser.isLL1Grammar();
  std::cout << "LL(1)状态: " << parser.getLL1Status() << std::endl;

  std::cout << std::endl;
}

void testSimpleExpression() {
  std::cout << "=== 测试简单表达式文法 ===" << std::endl;

  LL1Parser parser;
  std::string grammarStr = "S -> E\n"
                           "E -> T E1\n"
                           "E1 -> + T E1 | ε\n"
                           "T -> F T1\n"
                           "T1 -> * F T1 | ε\n"
                           "F -> id | ( E )";

  parser.readGrammarFromString(grammarStr);
  parser.setStartSymbol("S");

  std::cout << "文法:" << std::endl;
  parser.printGrammar();
  std::cout << std::endl;

  bool isLL1 = parser.isLL1Grammar();
  std::cout << "LL(1)状态: " << parser.getLL1Status() << std::endl;

  if (isLL1) {
    parser.buildParseTable();
    std::cout << std::endl;
    parser.printParseTable();

    // 测试语法分析
    std::cout << std::endl << "=== 语法分析测试 ===" << std::endl;
    auto result = parser.parse("id + id * id");
    if (result.success) {
      std::cout << "分析成功!" << std::endl;
    } else {
      std::cout << "分析失败: " << result.errorMessage << std::endl;
    }
  }

  std::cout << std::endl;
}

void testIfElseGrammar() {
  std::cout << "=== 测试if-else文法 ===" << std::endl;

  LL1Parser parser;
  std::string grammarStr = "S -> if E then S S' | other\n"
                           "S' -> else S | ε\n"
                           "E -> id";

  parser.readGrammarFromString(grammarStr);
  parser.setStartSymbol("S");

  std::cout << "文法:" << std::endl;
  parser.printGrammar();
  std::cout << std::endl;

  bool isLL1 = parser.isLL1Grammar();
  std::cout << "LL(1)状态: " << parser.getLL1Status() << std::endl;

  if (isLL1) {
    parser.buildParseTable();

    // 测试语法分析
    std::cout << std::endl << "=== 语法分析测试 ===" << std::endl;
    std::vector<std::string> testInputs = {"if id then other",
                                           "if id then other else other"};

    for (const auto &input : testInputs) {
      std::cout << "分析输入: " << input << std::endl;
      auto result = parser.parse(input);
      if (result.success) {
        std::cout << "分析成功!" << std::endl;
      } else {
        std::cout << "分析失败: " << result.errorMessage << std::endl;
      }
    }
  }

  std::cout << std::endl;
}

int main() {
  std::cout << "LL(1)文法判定与预测分析器测试程序" << std::endl;
  std::cout << "================================" << std::endl << std::endl;

  try {
    testLL1Grammar();
    testNewExample(); // 新增测试样例
    testNonLL1Grammar();
    testSimpleExpression();
    testIfElseGrammar();

    std::cout << "所有测试完成!" << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "测试过程中发生错误: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
