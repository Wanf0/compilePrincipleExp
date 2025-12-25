#include "../include/token.h"
#include <unordered_map>

std::string tokenTypeToString(TokenType type) {
  static const std::unordered_map<TokenType, std::string> tokenNames = {
      // 关键字
      {TokenType::INT, "INTTK"},
      {TokenType::VOID, "VOIDTK"},
      {TokenType::RETURN, "RETURNTK"},
      {TokenType::IF, "IFTK"},
      {TokenType::ELSE, "ELSETK"},
      {TokenType::WHILE, "WHILETK"},
      {TokenType::BREAK, "BREAKTK"},
      {TokenType::CONTINUE, "CONTINUETK"},
      {TokenType::CONST, "CONSTTK"},

      // 标识符
      {TokenType::IDENTIFIER, "ID"},

      // 常量
      {TokenType::INTEGER_CONST, "INTCON"},

      // 运算符
      {TokenType::PLUS, "PLUS"},
      {TokenType::MINUS, "MINUS"},
      {TokenType::MULTIPLY, "MULTIPLY"},
      {TokenType::DIVIDE, "DIVIDE"},
      {TokenType::MOD, "MOD"},
      {TokenType::ASSIGN, "ASSIGN"},
      {TokenType::EQ, "EQ"},
      {TokenType::NEQ, "NEQ"},
      {TokenType::LT, "LT"},
      {TokenType::LTE, "LTE"},
      {TokenType::GT, "GT"},
      {TokenType::GTE, "GTE"},
      {TokenType::AND, "AND"},
      {TokenType::OR, "OR"},
      {TokenType::NOT, "NOT"},

      // 分隔符
      {TokenType::SEMICOLON, "SEMICOLON"},
      {TokenType::COMMA, "COMMA"},
      {TokenType::LPAREN, "LPARENT"},
      {TokenType::RPAREN, "RPARENT"},
      {TokenType::LBRACKET, "LBRACKET"},
      {TokenType::RBRACKET, "RBRACKET"},
      {TokenType::LBRACE, "LBRACE"},
      {TokenType::RBRACE, "RBRACE"},

      // 特殊
      {TokenType::END_OF_FILE, "EOF"},
      {TokenType::ERROR, "ERROR"}};

  auto it = tokenNames.find(type);
  return it != tokenNames.end() ? it->second : "UNKNOWN";
}
