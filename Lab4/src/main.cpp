#include "../include/ast.h"
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/semantic.h"
#include "../include/token.h"
#include "error.h"
#include <iostream>
#include <vector>

void printUsage(const std::string &programName) {
  std::cout << "用法: " << programName << " <source_file.sy>" << std::endl;
  std::cout << "示例: " << programName << " test.sy" << std::endl;
}

int main(int argc, char *argv[]) {
  // 参数检查
  if (argc != 2) {
    printUsage(argv[0]);
    return 1;
  }

  std::string filename = argv[1];

  // 检查文件扩展名
  if (filename.length() < 3 ||
      filename.substr(filename.length() - 3) != ".sy") {
    std::cerr << "错误: 文件扩展名必须是 .sy" << std::endl;
    return 1;
  }

  try {
    // 初始化词法分析器
    Lexer lexer(filename);

    if (!lexer.isFileOpen()) {
      std::cerr << "错误: 无法打开文件 " << filename << std::endl;
      return 1;
    }

    // 重置错误处理器
    ErrorHandler::reset();

    // 收集所有token
    std::vector<Token> tokens;
    Token token;

    do {
      token = lexer.getNextToken();
      if (token.type != TokenType::ERROR &&
          token.type != TokenType::END_OF_FILE) {
        tokens.push_back(token);
      }
    } while (token.type != TokenType::END_OF_FILE);

    // 输出结果
    if (ErrorHandler::hasError()) {
      // 有词法错误，只输出错误信息
      return 0;
    } else {
      // 无词法错误，进行语法分析
      Parser parser(lexer, tokens);
      std::unique_ptr<ASTNode> ast = parser.parse();

      // 检查是否有语法错误
      if (ErrorHandler::hasError()) {
        // 有语法错误，错误信息已经输出
        return 0;
      } else {
        // 无语法错误，进行语义分析
        SemanticAnalyzer sem;
        sem.check(ast);

        // 如果没有语义错误，输出 success
        if (!sem.hasErrors()) {
          std::cout << "success" << std::endl;
        }
        // 无语法错误，输出AST
        if (ast) {
          ast->printPreOrder();
        }
      }
    }

  } catch (const std::exception &e) {
    std::cerr << "错误: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
