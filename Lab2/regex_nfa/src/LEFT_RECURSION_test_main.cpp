#include "../include/grammar.h"
#include <iostream>

int main() {
  std::vector<std::string> lines = {"E -> E + T | T", "T -> T * F | F",
                                    "F -> ( E ) | id"};

  Grammar G = Grammar::fromLines(lines);

  std::cout << "Original Grammar:\n";
  G.print();

  std::cout << "\nAfter Removing Left Recursion:\n";
  G.eliminateLeftRecursion();
  G.print();

  return 0;
}
