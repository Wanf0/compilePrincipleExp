#ifndef TOKEN_H
#define TOKEN_H

#include <string>

// Token类型枚举
enum class TokenType {
  // 关键字
  INT,
  VOID,
  FLOAT,
  RETURN,
  IF,
  ELSE,
  WHILE,
  BREAK,
  CONTINUE,
  CONST,

  // 标识符
  IDENTIFIER,

  // 常量
  INTEGER_CONST,
  FLOAT_CONST, // 新增：浮点常量

  // 运算符
  PLUS,
  MINUS,
  MULTIPLY,
  DIVIDE,
  MOD, // + - * / %
  ASSIGN,
  EQ,
  NEQ,
  LT,
  LTE,
  GT,
  GTE, // = == != < <= > >=
  AND,
  OR,
  NOT, // && || !

  // 分隔符
  SEMICOLON,
  COMMA, // ; ,
  LPAREN,
  RPAREN,
  LBRACKET,
  RBRACKET, // ( ) [ ]
  LBRACE,
  RBRACE, // { }

  // 特殊
  END_OF_FILE,
  ERROR
};

// Token结构体
struct Token {
  TokenType type;
  std::string lexeme; // 原始文本
  int line;           // 行号
  int value;          // 数值（仅用于整数常量）

  Token(TokenType t = TokenType::ERROR, std::string l = "", int ln = 1,
        int v = 0)
      : type(t), lexeme(l), line(ln), value(v) {}
};

// Token类型转字符串（用于输出）
std::string tokenTypeToString(TokenType type);

#endif
