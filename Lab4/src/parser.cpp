#include "../include/parser.h"
#define DEBUG
#include <iostream>

// Parser构造函数
Parser::Parser(Lexer &lexer, const std::vector<Token> &tokens)
    : lexer(lexer), tokens(tokens), tokenIndex(0) {
  if (!tokens.empty()) {
    currentToken = tokens[0];
  }
}

// Token管理函数
void Parser::consume() {
  tokenIndex++;
  if (tokenIndex < tokens.size()) {
    currentToken = tokens[tokenIndex];
  } else {
    currentToken.type = TokenType::END_OF_FILE;
    currentToken.lexeme = "";
  }
}

bool Parser::match(TokenType expected) {
  if (currentToken.type == expected) {
    consume();
    return true;
  }
  return false;
}

bool Parser::check(TokenType type) { return currentToken.type == type; }

TokenType Parser::peek() {
  if (tokenIndex + 1 < tokens.size()) {
    return tokens[tokenIndex + 1].type;
  }
  return TokenType::END_OF_FILE;
}

Token Parser::getToken() { return currentToken; }

// 错误处理函数
void Parser::reportSyntaxError(const std::string &message) {
  ErrorHandler::reportSyntaxError(currentToken.line, message);
}

void Parser::syncTo(TokenType syncToken) {
  while (currentToken.type != syncToken &&
         currentToken.type != TokenType::END_OF_FILE) {
    consume();
  }

  if (currentToken.type == syncToken) {
    consume();
  }
}

// 辅助函数
bool Parser::isUnaryOp() {
  return check(TokenType::PLUS) || check(TokenType::MINUS) ||
         check(TokenType::NOT);
}

bool Parser::isMulOp() {
  return check(TokenType::MULTIPLY) || check(TokenType::DIVIDE) ||
         check(TokenType::MOD);
}

bool Parser::isAddOp() {
  return check(TokenType::PLUS) || check(TokenType::MINUS);
}

bool Parser::isRelOp() {
  return check(TokenType::LT) || check(TokenType::LTE) ||
         check(TokenType::GT) || check(TokenType::GTE);
}

bool Parser::isEqOp() { return check(TokenType::EQ) || check(TokenType::NEQ); }

// 语法分析函数
std::unique_ptr<ASTNode> Parser::parseCompUnit() {
  int startLine = currentToken.line;
  auto node = std::make_unique<NonTerminalNode>("CompUnit", startLine);

  // CompUnit → {Decl} {FuncDef} MainFuncDef
  // 支持 INT / VOID / CONST 开头的声明或函数定义
  while (true) {
    if (check(TokenType::CONST) || check(TokenType::INT) ||
        check(TokenType::VOID)) {
      // 可能是声明或函数定义
      // 判断是否是函数定义：下下个 token 是否为 '('
      if (peek() == TokenType::LPAREN ||
          (peek() == TokenType::IDENTIFIER && tokenIndex + 2 < tokens.size() &&
           tokens[tokenIndex + 2].type == TokenType::LPAREN)) {
        // 函数定义
        auto funcDef = parseFuncDef();
        if (funcDef)
          node->addChild(std::move(funcDef));
      } else {
        // 声明
        auto decl = parseDecl();
        if (decl)
          node->addChild(std::move(decl));
      }
    } else {
      break;
    }
  }

  return node;
}

std::unique_ptr<ASTNode> Parser::parseDecl() {
  int startLine = currentToken.line;

  if (check(TokenType::CONST)) {
    return parseConstDecl();
  } else {
    return parseVarDecl();
  }
}

std::unique_ptr<ASTNode> Parser::parseConstDecl() {
  int startLine = currentToken.line;
  auto node = std::make_unique<NonTerminalNode>("ConstDecl", startLine);

  // ConstDecl → 'const' BType ConstDef { ',' ConstDef } ';'
  if (!match(TokenType::CONST)) {
    reportSyntaxError("缺少 'const'");
    return nullptr;
  }

  auto btype = parseBType();
  if (btype)
    node->addChild(std::move(btype));

  // 这里简化处理：解析至少一个 ConstDef（我们只取标识符和可选初始化）
  bool first = true;
  while (true) {
    if (!first) {
      if (!match(TokenType::COMMA))
        break;
    }
    first = false;

    if (!check(TokenType::IDENTIFIER)) {
      reportSyntaxError("缺少常量名");
      // 同步到分号并返回
      while (!check(TokenType::SEMICOLON) && !check(TokenType::END_OF_FILE))
        consume();
      break;
    }
    Token idTok = getToken();
    consume();
    auto idNode =
        std::make_unique<TerminalNode>("ID", idTok.lexeme, idTok.line);
    node->addChild(std::move(idNode));

    if (check(TokenType::ASSIGN)) {
      match(TokenType::ASSIGN);
      auto init = parseExp();
      if (init)
        node->addChild(std::move(init));
    }
  }

  if (!match(TokenType::SEMICOLON)) {
    reportSyntaxError("缺少 ';'");
  }

  return node;
}

std::unique_ptr<ASTNode> Parser::parseVarDecl() {
  int startLine = currentToken.line;
  auto node = std::make_unique<NonTerminalNode>("VarDecl", startLine);

  // VarDecl → BType VarDef { ',' VarDef } ';'
  auto btype = parseBType();
  if (btype)
    node->addChild(std::move(btype));

  // 解析至少一个 VarDef（至少要有标识符）
  bool first = true;
  while (true) {
    if (!first) {
      if (!match(TokenType::COMMA))
        break;
    }
    first = false;

    if (!check(TokenType::IDENTIFIER)) {
      reportSyntaxError("缺少变量名");
      // 同步到分号并返回
      while (!check(TokenType::SEMICOLON) && !check(TokenType::END_OF_FILE))
        consume();
      break;
    }

    // 读取变量名（在 consume 前保存）
    Token idTok = getToken();
    consume();
    auto idNode =
        std::make_unique<TerminalNode>("ID", idTok.lexeme, idTok.line);
    node->addChild(std::move(idNode));

    // 可选数组维度（简单处理：保存表达式作为子节点）
    while (check(TokenType::LBRACKET)) {
      match(TokenType::LBRACKET);
      auto idxExp = parseExp();
      if (idxExp)
        node->addChild(std::move(idxExp));
      if (!match(TokenType::RBRACKET)) {
        reportSyntaxError("缺少 ']'");
        break;
      }
    }

    // 可选初始化
    if (check(TokenType::ASSIGN)) {
      match(TokenType::ASSIGN);
      auto init = parseExp();
      if (init)
        node->addChild(std::move(init));
    }
  }

  if (!match(TokenType::SEMICOLON)) {
    reportSyntaxError("缺少 ';'");
  }

  return node;
}

std::unique_ptr<ASTNode> Parser::parseFuncDef() {
  int startLine = currentToken.line;
  auto node = std::make_unique<NonTerminalNode>("FuncDef", startLine);

  // FuncDef → FuncType ID '(' [FuncFParams] ')' Block
  auto funcType = parseFuncType();
  if (funcType)
    node->addChild(std::move(funcType));

  // 识别并保存函数名 token（在 consume 之前）
  if (!check(TokenType::IDENTIFIER)) {
    reportSyntaxError("缺少函数名");
    return nullptr;
  }
  Token idToken = getToken();
  consume();
  // 添加函数名节点（使用保存的 token）
  auto funcName =
      std::make_unique<TerminalNode>("ID", idToken.lexeme, idToken.line);
  node->addChild(std::move(funcName));

  if (!match(TokenType::LPAREN)) {
    reportSyntaxError("缺少 '('");
  }

  // 解析参数（当前版本仍然不支持完整参数解析）
  if (!check(TokenType::RPAREN)) {
    // TODO: 解析函数参数（后续实验）
    // 暂时先报错并返回 nullptr，不破坏括号结构
    reportSyntaxError("函数参数暂不支持");
    return nullptr;
  }
  if (!match(TokenType::RPAREN)) {
    reportSyntaxError("缺少 ')'");
  }

  auto block = parseBlock();
  if (block)
    node->addChild(std::move(block));

  return node;
}
// parseFuncType 示例片段：替换原来的 parseFuncType 函数
std::unique_ptr<ASTNode> Parser::parseFuncType() {
  int startLine = currentToken.line;
  auto node = std::make_unique<NonTerminalNode>("FuncType", startLine);

  if (check(TokenType::INT)) {
    auto intToken = getToken();
    auto intNode =
        std::make_unique<TerminalNode>("INTTK", intToken.lexeme, intToken.line);
    node->addChild(std::move(intNode));
    consume();
  } else if (check(TokenType::VOID)) {
    auto voidToken = getToken();
    auto voidNode = std::make_unique<TerminalNode>("VOIDTK", voidToken.lexeme,
                                                   voidToken.line);
    node->addChild(std::move(voidNode));
    consume();
  } else if (check(TokenType::FLOAT)) {
    auto floatToken = getToken();
    auto floatNode = std::make_unique<TerminalNode>(
        "FLOATTK", floatToken.lexeme, floatToken.line);
    node->addChild(std::move(floatNode));
    consume();
  } else {
    reportSyntaxError("缺少函数返回类型");
  }

  return node;
}

// parseBType 示例片段：替换原来的 parseBType 函数
std::unique_ptr<ASTNode> Parser::parseBType() {
  int startLine = currentToken.line;
  auto node = std::make_unique<NonTerminalNode>("BType", startLine);

  if (check(TokenType::INT)) {
    auto intToken = getToken();
    auto intNode =
        std::make_unique<TerminalNode>("INTTK", intToken.lexeme, intToken.line);
    node->addChild(std::move(intNode));
    consume();
  } else if (check(TokenType::FLOAT)) {
    auto floatToken = getToken();
    auto floatNode = std::make_unique<TerminalNode>(
        "FLOATTK", floatToken.lexeme, floatToken.line);
    node->addChild(std::move(floatNode));
    consume();
  } else {
    reportSyntaxError("缺少类型说明");
  }

  return node;
}

std::unique_ptr<ASTNode> Parser::parseBlock() {
  int startLine = currentToken.line;
  auto node = std::make_unique<NonTerminalNode>("Block", startLine);

  if (!match(TokenType::LBRACE)) {
    reportSyntaxError("缺少 '{'");
    return nullptr;
  }

  // Block → '{' {BlockItem} '}'
  while (!check(TokenType::RBRACE) && !check(TokenType::END_OF_FILE)) {
    auto blockItem = parseBlockItem();
    if (blockItem)
      node->addChild(std::move(blockItem));
  }

  if (!match(TokenType::RBRACE)) {
    reportSyntaxError("缺少 '}'");
  }

  return node;
}

std::unique_ptr<ASTNode> Parser::parseBlockItem() {
  int startLine = currentToken.line;

  // BlockItem → Decl | Stmt
  if (check(TokenType::CONST) || check(TokenType::INT) ||
      check(TokenType::VOID)) {
    // 可能是声明或函数调用表达式
    if (peek() == TokenType::LPAREN ||
        (peek() == TokenType::IDENTIFIER && tokenIndex + 2 < tokens.size() &&
         tokens[tokenIndex + 2].type == TokenType::LPAREN)) {
      // 函数调用表达式语句
      return parseStmt();
    } else {
      return parseDecl();
    }
  } else {
    return parseStmt();
  }
}

std::unique_ptr<ASTNode> Parser::parseStmt() {
  int startLine = currentToken.line;

  // Stmt → LVal '=' Exp ';'
  //      | [Exp] ';'
  //      | Block
  //      | 'if' '(' Cond ')' Stmt ['else' Stmt]
  //      | 'while' '(' Cond ')' Stmt
  //      | 'break' ';'
  //      | 'continue' ';'
  //      | 'return' [Exp] ';'

  if (check(TokenType::IF)) {
    auto node = std::make_unique<NonTerminalNode>("IfStmt", startLine);

    match(TokenType::IF); // 消费'if'

    if (!match(TokenType::LPAREN)) {
      reportSyntaxError("缺少 '('");
    }

    auto cond = parseCond();
    if (cond)
      node->addChild(std::move(cond));

    if (!match(TokenType::RPAREN)) {
      reportSyntaxError("缺少 ')'");
    }

    auto thenStmt = parseStmt();
    if (thenStmt)
      node->addChild(std::move(thenStmt));

    if (check(TokenType::ELSE)) {
      match(TokenType::ELSE);
      auto elseStmt = parseStmt();
      if (elseStmt)
        node->addChild(std::move(elseStmt));
    }

    return node;
  } else if (check(TokenType::WHILE)) {
    auto node = std::make_unique<NonTerminalNode>("WhileStmt", startLine);

    match(TokenType::WHILE);

    if (!match(TokenType::LPAREN)) {
      reportSyntaxError("缺少 '('");
    }

    auto cond = parseCond();
    if (cond)
      node->addChild(std::move(cond));

    if (!match(TokenType::RPAREN)) {
      reportSyntaxError("缺少 ')'");
    }

    auto stmt = parseStmt();
    if (stmt)
      node->addChild(std::move(stmt));

    return node;
  } else if (check(TokenType::BREAK)) {
    auto node = std::make_unique<NonTerminalNode>("BreakStmt", startLine);

    match(TokenType::BREAK);

    if (!match(TokenType::SEMICOLON)) {
      reportSyntaxError("缺少 ';'");
    }

    return node;
  } else if (check(TokenType::CONTINUE)) {
    auto node = std::make_unique<NonTerminalNode>("ContinueStmt", startLine);

    match(TokenType::CONTINUE);

    if (!match(TokenType::SEMICOLON)) {
      reportSyntaxError("缺少 ';'");
    }

    return node;
  } else if (check(TokenType::RETURN)) {
    auto node = std::make_unique<NonTerminalNode>("ReturnStmt", startLine);

    match(TokenType::RETURN);

    // 可选表达式
    if (!check(TokenType::SEMICOLON)) {
      auto exp = parseExp();
      if (exp)
        node->addChild(std::move(exp));
    }

    if (!match(TokenType::SEMICOLON)) {
      reportSyntaxError("缺少 ';'");
    }

    return node;
  } else if (check(TokenType::LBRACE)) {
    // 块语句
    return parseBlock();
  } else if (check(TokenType::IDENTIFIER)) {
    // 保存当前位置，以便回退
    size_t savedIndex = tokenIndex;
    Token savedToken = currentToken;

    // 尝试解析LVal
    auto lval = parseLVal();

    // 如果成功解析了LVal，并且当前token是等号，那么是赋值语句
    if (lval && check(TokenType::ASSIGN)) {
      // 是赋值语句
      auto node = std::make_unique<NonTerminalNode>("AssignStmt", startLine);
      node->addChild(std::move(lval));

      match(TokenType::ASSIGN);

      auto exp = parseExp();
      if (exp)
        node->addChild(std::move(exp));

      if (!match(TokenType::SEMICOLON)) {
        reportSyntaxError("缺少 ';'");
      }

      return node;
    } else {
      // 不是赋值语句，回退
      tokenIndex = savedIndex;
      currentToken = savedToken;
    }
  }

  // 如果不是上述语句，按表达式语句处理
  auto node = std::make_unique<NonTerminalNode>("ExpStmt", startLine);

  // 如果不是直接以分号开头，尝试解析表达式
  if (!check(TokenType::SEMICOLON)) {
    auto exp = parseExp();
    if (exp)
      node->addChild(std::move(exp));
  }

  // 必须有分号
  if (!match(TokenType::SEMICOLON)) {
    reportSyntaxError("缺少 ';'");
  }

  return node;
}

std::unique_ptr<ASTNode> Parser::parseExp() {
  // Exp → AddExp
  return parseLOrExp();
}

std::unique_ptr<ASTNode> Parser::parseCond() {
// Cond → LOrExp
#ifdef DEBUG
  std::cout << "DEBUG parseCond: 开始解析条件表达式，当前token: "
            << tokenTypeToString(currentToken.type) << " '"
            << currentToken.lexeme << "'" << std::endl;
#endif

  auto result = parseLOrExp();

#ifdef DEBUG
  std::cout << "DEBUG parseCond: 解析完成，当前token: "
            << tokenTypeToString(currentToken.type) << " '"
            << currentToken.lexeme << "'" << std::endl;
#endif

  return result;
}

std::unique_ptr<ASTNode> Parser::parseLVal() {
  int startLine = currentToken.line;
  auto node = std::make_unique<NonTerminalNode>("LVal", startLine);

  if (!check(TokenType::IDENTIFIER)) {
    reportSyntaxError("缺少标识符");
    return nullptr;
  }

  // 在消费前保存 identifier token
  Token idToken = getToken();
  consume();

  // 添加标识符节点（使用保存的 token）
  auto idNode =
      std::make_unique<TerminalNode>("ID", idToken.lexeme, idToken.line);
  node->addChild(std::move(idNode));

  // 检查是否有数组访问
  while (check(TokenType::LBRACKET)) {
    match(TokenType::LBRACKET);
    auto exp = parseExp();
    if (exp)
      node->addChild(std::move(exp));

    if (!match(TokenType::RBRACKET)) {
      reportSyntaxError("缺少 ']'");
    }
  }

  return node;
}

std::unique_ptr<ASTNode> Parser::parsePrimaryExp() {
  if (check(TokenType::LPAREN)) {
    match(TokenType::LPAREN);

#ifdef DEBUG
    std::cout << "DEBUG parsePrimaryExp: 遇到左括号，开始解析内部表达式"
              << std::endl;
#endif

    auto exp = parseLOrExp(); // 关键修改：使用 parseLOrExp

#ifdef DEBUG
    std::cout
        << "DEBUG parsePrimaryExp: 内部表达式解析完成，期望右括号，当前token: "
        << tokenTypeToString(currentToken.type) << " '" << currentToken.lexeme
        << "'" << std::endl;
#endif

    if (!match(TokenType::RPAREN)) {
      reportSyntaxError("缺少 ')'");
    }

    return exp;
  } else if (check(TokenType::IDENTIFIER) && peek() == TokenType::LPAREN) {
    // 函数调用
    return parseFuncCall();
  } else if (check(TokenType::IDENTIFIER)) {
    // LVal
    return parseLVal();
  } else {
    // Number
    return parseNumber();
  }
}

std::unique_ptr<ASTNode> Parser::parseNumber() {
  if (check(TokenType::INTEGER_CONST)) {
    auto token = getToken();
    auto node = std::make_unique<TerminalNode>("INTCON", token.lexeme,
                                               token.line, token.value);
    consume();
    return node;
  }
  reportSyntaxError("缺少数字常量");
  return nullptr;
}

std::unique_ptr<ASTNode> Parser::parseUnaryExp() {
  int startLine = currentToken.line;

  if (isUnaryOp()) {
    // UnaryOp UnaryExp
    auto node = std::make_unique<NonTerminalNode>("UnaryExp", startLine);

    auto unaryOp = parseUnaryOp();
    if (unaryOp)
      node->addChild(std::move(unaryOp));

    auto unaryExp = parseUnaryExp();
    if (unaryExp)
      node->addChild(std::move(unaryExp));

    return node;
  } else {
    // PrimaryExp
    return parsePrimaryExp();
  }
}

std::unique_ptr<ASTNode> Parser::parseUnaryOp() {
  int startLine = currentToken.line;
  auto node = std::make_unique<NonTerminalNode>("UnaryOp", startLine);

  if (check(TokenType::PLUS)) {
    auto token = getToken();
    auto opNode =
        std::make_unique<TerminalNode>("PLUS", token.lexeme, token.line);
    node->addChild(std::move(opNode));
    consume();
  } else if (check(TokenType::MINUS)) {
    auto token = getToken();
    auto opNode =
        std::make_unique<TerminalNode>("MINUS", token.lexeme, token.line);
    node->addChild(std::move(opNode));
    consume();
  } else if (check(TokenType::NOT)) {
    auto token = getToken();
    auto opNode =
        std::make_unique<TerminalNode>("NOT", token.lexeme, token.line);
    node->addChild(std::move(opNode));
    consume();
  } else {
    reportSyntaxError("缺少一元运算符");
    return nullptr;
  }

  return node;
}

std::unique_ptr<ASTNode> Parser::parseFuncCall() {
  int startLine = currentToken.line;
  auto node = std::make_unique<NonTerminalNode>("FuncCall", startLine);

  if (!check(TokenType::IDENTIFIER)) {
    reportSyntaxError("缺少函数名");
    return nullptr;
  }

  // 在消费之前保存函数名 token
  Token idToken = getToken();
  consume();

  // 添加函数名
  auto funcName =
      std::make_unique<TerminalNode>("ID", idToken.lexeme, idToken.line);
  node->addChild(std::move(funcName));

  if (!match(TokenType::LPAREN)) {
    reportSyntaxError("缺少 '('");
  }

  // 解析函数调用参数（支持 0 个或多个以逗号分隔的表达式）
  if (!check(TokenType::RPAREN)) {
    while (true) {
      auto arg = parseExp();
      if (arg)
        node->addChild(std::move(arg));

      if (check(TokenType::COMMA)) {
        match(TokenType::COMMA);
        continue;
      } else {
        break;
      }
    }
  }

  if (!match(TokenType::RPAREN)) {
    reportSyntaxError("缺少 ')'");
  }

  return node;
}

std::unique_ptr<ASTNode> Parser::parseMulExp() {
  auto node = parseUnaryExp();
  int startLine = currentToken.line;

  while (isMulOp()) {
    auto opNode = std::make_unique<NonTerminalNode>("BinaryExp", startLine);
    opNode->addChild(std::move(node));

    // 添加操作符
    auto token = getToken();
    std::string opName;
    if (token.type == TokenType::MULTIPLY)
      opName = "MULTIPLY";
    else if (token.type == TokenType::DIVIDE)
      opName = "DIVIDE";
    else
      opName = "MOD";

    auto opTerminal =
        std::make_unique<TerminalNode>(opName, token.lexeme, token.line);
    opNode->addChild(std::move(opTerminal));
    consume();

    auto right = parseUnaryExp();
    if (right)
      opNode->addChild(std::move(right));

    node = std::move(opNode);
  }

  return node;
}

std::unique_ptr<ASTNode> Parser::parseAddExp() {
  auto node = parseMulExp();
  int startLine = currentToken.line;

  while (isAddOp()) {
    auto opNode = std::make_unique<NonTerminalNode>("BinaryExp", startLine);
    opNode->addChild(std::move(node));

    // 添加操作符
    auto token = getToken();
    std::string opName = (token.type == TokenType::PLUS) ? "PLUS" : "MINUS";
    auto opTerminal =
        std::make_unique<TerminalNode>(opName, token.lexeme, token.line);
    opNode->addChild(std::move(opTerminal));
    consume();

    auto right = parseMulExp();
    if (right)
      opNode->addChild(std::move(right));

    node = std::move(opNode);
  }

  return node;
}

std::unique_ptr<ASTNode> Parser::parseRelExp() {
  auto node = parseAddExp();
  int startLine = currentToken.line;

  while (isRelOp()) {
    auto opNode = std::make_unique<NonTerminalNode>("RelExp", startLine);
    opNode->addChild(std::move(node));

    // 添加操作符
    auto token = getToken();
    std::string opName;
    switch (token.type) {
    case TokenType::LT:
      opName = "LT";
      break;
    case TokenType::LTE:
      opName = "LTE";
      break;
    case TokenType::GT:
      opName = "GT";
      break;
    case TokenType::GTE:
      opName = "GTE";
      break;
    default:
      opName = "UNKNOWN";
    }

    auto opTerminal =
        std::make_unique<TerminalNode>(opName, token.lexeme, token.line);
    opNode->addChild(std::move(opTerminal));
    consume();

    auto right = parseAddExp();
    if (right)
      opNode->addChild(std::move(right));

    node = std::move(opNode);
  }

  return node;
}

std::unique_ptr<ASTNode> Parser::parseEqExp() {
  auto node = parseRelExp();
  int startLine = currentToken.line;

  while (isEqOp()) {
    auto opNode = std::make_unique<NonTerminalNode>("EqExp", startLine);
    opNode->addChild(std::move(node));

    // 添加操作符
    auto token = getToken();
    std::string opName = (token.type == TokenType::EQ) ? "EQ" : "NEQ";
    auto opTerminal =
        std::make_unique<TerminalNode>(opName, token.lexeme, token.line);
    opNode->addChild(std::move(opTerminal));
    consume();

    auto right = parseRelExp();
    if (right)
      opNode->addChild(std::move(right));

    node = std::move(opNode);
  }

  return node;
}

std::unique_ptr<ASTNode> Parser::parseLAndExp() {
  auto node = parseEqExp();
  int startLine = currentToken.line;

  while (check(TokenType::AND)) {
    auto opNode = std::make_unique<NonTerminalNode>("LAndExp", startLine);
    opNode->addChild(std::move(node));

    // 添加操作符
    auto token = getToken();
    auto opTerminal =
        std::make_unique<TerminalNode>("AND", token.lexeme, token.line);
    opNode->addChild(std::move(opTerminal));
    consume();

    auto right = parseEqExp();
    if (right)
      opNode->addChild(std::move(right));

    node = std::move(opNode);
  }

  return node;
}

std::unique_ptr<ASTNode> Parser::parseLOrExp() {
  auto node = parseLAndExp();
  int startLine = currentToken.line;

  while (check(TokenType::OR)) {
    auto opNode = std::make_unique<NonTerminalNode>("LOrExp", startLine);
    opNode->addChild(std::move(node));

    // 添加操作符
    auto token = getToken();
    auto opTerminal =
        std::make_unique<TerminalNode>("OR", token.lexeme, token.line);
    opNode->addChild(std::move(opTerminal));
    consume();

    auto right = parseLAndExp();
    if (right)
      opNode->addChild(std::move(right));

    node = std::move(opNode);
  }

  return node;
}

// 主解析函数
std::unique_ptr<ASTNode> Parser::parse() {
  if (tokens.empty()) {
    return nullptr;
  }

  auto ast = parseCompUnit();

  // 检查是否消耗完所有Token
  if (currentToken.type != TokenType::END_OF_FILE) {
    reportSyntaxError("程序结束后有多余内容");
  }

  return ast;
}

std::unique_ptr<ASTNode> Parser::getAST() { return parse(); }

void Parser::printAST() {
  auto ast = parse();
  if (ast) {
    ast->printPreOrder();
  }
}
