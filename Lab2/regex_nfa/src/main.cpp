#include "../include/dfa_minimizer.h"
#include "../include/nfa_to_dfa.h"
#include "../include/thompson.h"
#include <fstream>
#include <iostream>

using namespace regexnfa;

int main() {
  std::string regex;
  std::cout << "Enter regex: ";
  std::getline(std::cin, regex);

  try {
    // 构建NFA
    NFA nfa = ThompsonBuilder::build_from_regex(regex);
    std::cout << "\n--- NFA ---\n";
    std::cout << nfa.to_dot() << std::endl;

    // 转换为DFA
    DFA dfa = NFAtoDFA::convert(nfa);
    std::cout << "\n--- Original DFA ---\n";
    dfa.print();

    // 最小化DFA
    DFA minimized_dfa = DFAMinimizer::minimize(dfa);
    std::cout << "\n--- Minimized DFA ---\n";
    minimized_dfa.print();

    // 保存原始DFA和最小化DFA的图形
    std::ofstream orig_file("dfa_original.dot");
    orig_file << dfa.to_dot();
    orig_file.close();

    std::ofstream min_file("dfa_minimized.dot");
    min_file << minimized_dfa.to_dot();
    min_file.close();

    std::cout << "\nDFA DOT files saved:\n";
    std::cout << "  Original: dfa_original.dot\n";
    std::cout << "  Minimized: dfa_minimized.dot\n";

    // 生成PNG图片
    system("dot -Tpng dfa_original.dot -o dfa_original.png");
    system("dot -Tpng dfa_minimized.dot -o dfa_minimized.png");
    std::cout << "PNG images generated:\n";
    std::cout << "  Original: dfa_original.png\n";
    std::cout << "  Minimized: dfa_minimized.png\n";

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
