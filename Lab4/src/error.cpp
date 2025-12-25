#include "../include/error.h"
#include <iostream>

int ErrorHandler::errorCount = 0;
int ErrorHandler::lastErrorLine = -1;
ErrorType ErrorHandler::lastErrorType = ErrorType::A;

void ErrorHandler::reportLexicalError(int line, const std::string &msg) {
  // 同一行只报告一个错误
  if (line == lastErrorLine) {
    return;
  }

  lastErrorLine = line;
  lastErrorType = ErrorType::A;
  errorCount++;

  std::cout << "Error type A at line " << line << ": " << msg << std::endl;
}

void ErrorHandler::reportSyntaxError(int line, const std::string &msg) {
  // 同一行只报告一个错误
  if (line == lastErrorLine && lastErrorType == ErrorType::B) {
    return;
  }

  lastErrorLine = line;
  lastErrorType = ErrorType::B;
  errorCount++;

  std::cout << "Error type B at line " << line << ": " << msg << std::endl;
}

void ErrorHandler::reset() {
  errorCount = 0;
  lastErrorLine = -1;
  lastErrorType = ErrorType::A;
}

bool ErrorHandler::hasError() { return errorCount > 0; }

bool ErrorHandler::hasLexicalError() {
  return errorCount > 0 && lastErrorType == ErrorType::A;
}

int ErrorHandler::getErrorCount() { return errorCount; }
