#ifndef LEXER_H
#define LEXER_H

#include "error.h"
#include "token.h"
#include <fstream>
#include <string>

class Lexer {
private:
  std::ifstream sourceFile; // 源文件流
  std::string fileName;     // 文件名
  int currentLine;          // 当前行号
  int currentPos;           // 当前行位置
  char currentChar;         // 当前字符
  bool eofReached;          // 是否到达文件末尾

  // 字符读取
  void advance();
  char peek();
  void skipWhitespace();
  void skipComment();

  // Token识别函数
  Token recognizeKeywordOrIdentifier();
  Token recognizeNumber();
  Token recognizeOperator();

  // 辅助函数
  bool isKeyword(const std::string &str);
  TokenType getKeywordType(const std::string &str);
  bool isValidIdentifierChar(char c, bool firstChar = false);
  bool isPotentialOperator(char c);

public:
  Lexer(const std::string &filename);
  ~Lexer();

  // 主接口：获取下一个Token
  Token getNextToken();

  // 检查文件是否打开成功
  bool isFileOpen() const;

  // 获取当前行号
  int getCurrentLine() const;
};

#endif
