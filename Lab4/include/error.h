#ifndef ERROR_H
#define ERROR_H

#include <string>

enum class ErrorType {
  A = 1, // 词法错误
  B = 2, // 语法错误
  // 预留语义错误
  C = 3
};

class ErrorHandler {
private:
  static int errorCount;
  static int lastErrorLine;
  static ErrorType lastErrorType;

public:
  // 报告词法错误
  static void reportLexicalError(int line, const std::string &msg);

  // 报告语法错误
  static void reportSyntaxError(int line, const std::string &msg);

  // 重置错误状态
  static void reset();

  // 检查是否有任何错误
  static bool hasError();

  // 检查是否有词法错误
  static bool hasLexicalError();

  // 获取错误计数
  static int getErrorCount();
};

#endif
