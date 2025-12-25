#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "error.h"
#include "lexer.h"
#include <memory>
#include <vector>

class Parser {
private:
  Lexer &lexer;
  Token currentToken;
  std::vector<Token> tokens;
  size_t tokenIndex;

  // Token管理
  void consume();
  bool match(TokenType expected);
  bool check(TokenType type);
  TokenType peek();
  Token getToken();

  // 错误处理
  void reportSyntaxError(const std::string &message);
  void syncTo(TokenType syncToken);

  // 语法分析函数
  std::unique_ptr<ASTNode> parseCompUnit();
  std::unique_ptr<ASTNode> parseDecl();
  std::unique_ptr<ASTNode> parseConstDecl();
  std::unique_ptr<ASTNode> parseVarDecl();
  std::unique_ptr<ASTNode> parseFuncDef();
  std::unique_ptr<ASTNode> parseFuncType();
  std::unique_ptr<ASTNode> parseBlock();
  std::unique_ptr<ASTNode> parseBlockItem();
  std::unique_ptr<ASTNode> parseStmt();
  std::unique_ptr<ASTNode> parseExp();
  std::unique_ptr<ASTNode> parseCond();
  std::unique_ptr<ASTNode> parseLVal();
  std::unique_ptr<ASTNode> parsePrimaryExp();
  std::unique_ptr<ASTNode> parseNumber();
  std::unique_ptr<ASTNode> parseUnaryExp();
  std::unique_ptr<ASTNode> parseUnaryOp();
  std::unique_ptr<ASTNode> parseFuncCall();
  std::unique_ptr<ASTNode> parseMulExp();
  std::unique_ptr<ASTNode> parseAddExp();
  std::unique_ptr<ASTNode> parseRelExp();
  std::unique_ptr<ASTNode> parseEqExp();
  std::unique_ptr<ASTNode> parseLAndExp();
  std::unique_ptr<ASTNode> parseLOrExp();
  std::unique_ptr<ASTNode> parseConstExp();
  std::unique_ptr<ASTNode> parseConstInitVal();
  std::unique_ptr<ASTNode> parseInitVal();
  std::unique_ptr<ASTNode> parseBType();

  // 辅助函数
  bool isUnaryOp();
  bool isMulOp();
  bool isAddOp();
  bool isRelOp();
  bool isEqOp();

public:
  Parser(Lexer &lexer, const std::vector<Token> &tokens);

  // 主解析函数
  std::unique_ptr<ASTNode> parse();

  // 获取AST根节点
  std::unique_ptr<ASTNode> getAST();

  // 打印AST
  void printAST();
};

#endif
