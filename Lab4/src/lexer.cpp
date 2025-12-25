#include "../include/lexer.h"
#include <cctype>
#include <stdexcept>
#include <unordered_map>

// 关键字映射表
static const std::unordered_map<std::string, TokenType> KEYWORDS = {
    {"int", TokenType::INT},
    {"void", TokenType::VOID},
    {"float", TokenType::FLOAT},
    {"return", TokenType::RETURN},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"while", TokenType::WHILE},
    {"break", TokenType::BREAK},
    {"continue", TokenType::CONTINUE},
    {"const", TokenType::CONST}};

// 运算符映射表
static const std::unordered_map<std::string, TokenType> OPERATORS = {
    {"+", TokenType::PLUS},     {"-", TokenType::MINUS},
    {"*", TokenType::MULTIPLY}, {"/", TokenType::DIVIDE},
    {"%", TokenType::MOD},      {"=", TokenType::ASSIGN},
    {"==", TokenType::EQ},      {"!=", TokenType::NEQ},
    {"<", TokenType::LT},       {"<=", TokenType::LTE},
    {">", TokenType::GT},       {">=", TokenType::GTE},
    {"&&", TokenType::AND},     {"||", TokenType::OR},
    {"!", TokenType::NOT}};

// 分隔符映射表
static const std::unordered_map<char, TokenType> SEPARATORS = {
    {';', TokenType::SEMICOLON}, {',', TokenType::COMMA},
    {'(', TokenType::LPAREN},    {')', TokenType::RPAREN},
    {'[', TokenType::LBRACKET},  {']', TokenType::RBRACKET},
    {'{', TokenType::LBRACE},    {'}', TokenType::RBRACE}};

// Lexer实现

Lexer::Lexer(const std::string &filename)
    : fileName(filename), currentLine(1), currentPos(0), currentChar(' '),
      eofReached(false) {
  sourceFile.open(filename);
  if (!sourceFile.is_open()) {
    throw std::runtime_error("无法打开文件: " + filename);
  }
}

Lexer::~Lexer() {
  if (sourceFile.is_open()) {
    sourceFile.close();
  }
}

bool Lexer::isFileOpen() const { return sourceFile.is_open(); }

int Lexer::getCurrentLine() const { return currentLine; }

void Lexer::advance() {
  if (sourceFile.eof()) {
    eofReached = true;
    currentChar = '\0';
    return;
  }

  sourceFile.get(currentChar);
  currentPos++;

  if (currentChar == '\n') {
    currentLine++;
    currentPos = 0;
  }
}

char Lexer::peek() {
  if (sourceFile.eof()) {
    return '\0';
  }
  return sourceFile.peek();
}

void Lexer::skipWhitespace() {
  while (!eofReached && std::isspace(currentChar)) {
    advance();
  }
}

void Lexer::skipComment() {
  // 单行注释
  if (currentChar == '/' && peek() == '/') {
    while (!eofReached && currentChar != '\n') {
      advance();
    }
    if (!eofReached) {
      advance(); // 跳过换行符
      skipWhitespace();
    }
  }
  // 多行注释
  else if (currentChar == '/' && peek() == '*') {
    advance(); // 跳过 '/'
    advance(); // 跳过 '*'

    while (!eofReached) {
      if (currentChar == '*' && peek() == '/') {
        advance(); // 跳过 '*'
        advance(); // 跳过 '/'
        break;
      }
      advance();
    }

    if (!eofReached) {
      skipWhitespace();
    }
  }
}

bool Lexer::isKeyword(const std::string &str) {
  return KEYWORDS.find(str) != KEYWORDS.end();
}

TokenType Lexer::getKeywordType(const std::string &str) {
  auto it = KEYWORDS.find(str);
  return it != KEYWORDS.end() ? it->second : TokenType::IDENTIFIER;
}

bool Lexer::isValidIdentifierChar(char c, bool firstChar) {
  if (firstChar) {
    return std::isalpha(c) || c == '_';
  }
  return std::isalnum(c) || c == '_';
}

Token Lexer::recognizeKeywordOrIdentifier() {
  int startLine = currentLine;
  std::string lexeme;

  lexeme += currentChar;
  advance();

  while (!eofReached && isValidIdentifierChar(currentChar, false)) {
    lexeme += currentChar;
    advance();
  }

  TokenType type = getKeywordType(lexeme);
  return Token(type, lexeme, startLine);
}

// Token Lexer::recognizeNumber() {
//   int startLine = currentLine;
//   std::string lexeme;
//
//   // 处理0的情况
//   if (currentChar == '0') {
//     lexeme += currentChar;
//     advance();
//
//     // 十六进制
//     if (currentChar == 'x' || currentChar == 'X') {
//       lexeme += currentChar;
//       advance();
//
//       bool hasDigit = false;
//       while (!eofReached && std::isxdigit(currentChar)) {
//         hasDigit = true;
//         lexeme += currentChar;
//
//         // 检查非法十六进制字符
//         if (!(std::isdigit(currentChar) ||
//               (currentChar >= 'a' && currentChar <= 'f') ||
//               (currentChar >= 'A' && currentChar <= 'F'))) {
//           ErrorHandler::reportLexicalError(startLine,
//                                            "非法十六进制数: '" + lexeme +
//                                            "'");
//           return Token(TokenType::ERROR, lexeme, startLine);
//         }
//         advance();
//       }
//
//       if (!hasDigit) {
//         ErrorHandler::reportLexicalError(startLine, "十六进制数缺少数字: '" +
//                                                         lexeme + "'");
//         return Token(TokenType::ERROR, lexeme, startLine);
//       }
//
//       // 转换十六进制数为十进制
//       int value = std::stoi(lexeme.substr(2), nullptr, 16);
//       return Token(TokenType::INTEGER_CONST, lexeme, startLine, value);
//     }
//     // 八进制或0
//     else {
//       bool isOctal = false;
//       while (!eofReached && std::isdigit(currentChar)) {
//         if (currentChar >= '8') {
//           ErrorHandler::reportLexicalError(
//               startLine, "非法八进制数: '" + lexeme + currentChar + "'");
//           return Token(TokenType::ERROR, lexeme, startLine);
//         }
//         isOctal = true;
//         lexeme += currentChar;
//         advance();
//       }
//       (void)isOctal; // 避免未使用变量警告
//
//       if (lexeme == "0") {
//         return Token(TokenType::INTEGER_CONST, lexeme, startLine, 0);
//       }
//
//       // 转换八进制数为十进制
//       int value = std::stoi(lexeme, nullptr, 8);
//       return Token(TokenType::INTEGER_CONST, lexeme, startLine, value);
//     }
//   }
//   // 十进制
//   else {
//     while (!eofReached && std::isdigit(currentChar)) {
//       lexeme += currentChar;
//       advance();
//     }
//
//     int value = std::stoi(lexeme);
//     return Token(TokenType::INTEGER_CONST, lexeme, startLine, value);
//   }
// }
Token Lexer::recognizeNumber() {
  int startLine = currentLine;
  std::string lexeme;

  // 读取整数部分（至少一位）
  while (!eofReached && std::isdigit(currentChar)) {
    lexeme += currentChar;
    advance();
  }

  // 如果遇到小数点，尝试解析浮点数（要求小数点后至少有一位数字）
  if (!eofReached && currentChar == '.') {
    char next = peek();
    if (std::isdigit(next)) {
      // 这是一个浮点数字面量
      lexeme += currentChar; // 添加 '.'
      advance();             // 跳过 '.'
      bool hasFrac = false;
      while (!eofReached && std::isdigit(currentChar)) {
        hasFrac = true;
        lexeme += currentChar;
        advance();
      }
      if (!hasFrac) {
        // 小数点后没有数字 —— 视为词法错误
        ErrorHandler::reportLexicalError(startLine,
                                         "非法浮点数: '" + lexeme + "'");
        return Token(TokenType::ERROR, lexeme, startLine);
      }
      // 返回浮点数 token（保留 lexeme，value 字段不使用）
      return Token(TokenType::FLOAT_CONST, lexeme, startLine);
    } else {
      // 遇到 '.' 但后面不是数字：小数点不能作为其他用途，视为词法错误
      // 不要消费 '.', 让上层处理（这里我们报告词法错误并消费）
      std::string dot(1, currentChar);
      ErrorHandler::reportLexicalError(startLine, "非法字符: '" + dot + "'");
      advance();
      return Token(TokenType::ERROR, dot, startLine);
    }
  }

  // 没有小数点：处理八/十六进制的原实现逻辑（保留之前对 0x / 八进制 的处理）
  // 处理以 0 开头的特殊情况
  if (lexeme.size() > 0 && lexeme[0] == '0') {
    // 已经读了至少 '0'
    // 如果下一个字符是 x 或 X 并且我们还没有走到 EOF，则处理十六进制
    if (!eofReached && (currentChar == 'x' || currentChar == 'X')) {
      lexeme += currentChar;
      advance();
      bool hasDigit = false;
      while (!eofReached && std::isxdigit(currentChar)) {
        hasDigit = true;
        lexeme += currentChar;
        advance();
      }
      if (!hasDigit) {
        ErrorHandler::reportLexicalError(startLine, "十六进制数缺少数字: '" +
                                                        lexeme + "'");
        return Token(TokenType::ERROR, lexeme, startLine);
      }
      int value = std::stoi(lexeme.substr(2), nullptr, 16);
      return Token(TokenType::INTEGER_CONST, lexeme, startLine, value);
    } else {
      // 处理可能的八进制（如果后续是数字）
      bool isOctal = false;
      std::string octLex = lexeme;
      while (!eofReached && std::isdigit(currentChar)) {
        if (currentChar >= '8') {
          ErrorHandler::reportLexicalError(
              startLine, "非法八进制数: '" + octLex + currentChar + "'");
          return Token(TokenType::ERROR, octLex + currentChar, startLine);
        }
        isOctal = true;
        octLex += currentChar;
        advance();
      }
      if (octLex == "0") {
        return Token(TokenType::INTEGER_CONST, octLex, startLine, 0);
      }
      int value = std::stoi(octLex, nullptr, 8);
      return Token(TokenType::INTEGER_CONST, octLex, startLine, value);
    }
  }

  // 十进制整数（常规）
  if (!lexeme.empty()) {
    int value = std::stoi(lexeme);
    return Token(TokenType::INTEGER_CONST, lexeme, startLine, value);
  }

  // 如果没有读到任何数字（应该不会到这里），报告错误
  ErrorHandler::reportLexicalError(startLine, "非法数字字面量");
  return Token(TokenType::ERROR, lexeme, startLine);
}

Token Lexer::recognizeOperator() {
  int startLine = currentLine;
  std::string lexeme;

  // 先保存第一个字符
  char firstChar = currentChar;
  char secondChar = peek();
  std::string twoCharOp =
      std::string(1, firstChar) + std::string(1, secondChar);

  // 先检查双字符运算符
  auto it = OPERATORS.find(twoCharOp);
  if (it != OPERATORS.end()) {
    // 是合法的双字符运算符
    lexeme = twoCharOp;
    advance(); // 消费第一个字符
    advance(); // 消费第二个字符
    return Token(it->second, lexeme, startLine);
  }

  // 再检查单字符运算符
  std::string oneCharOp = std::string(1, firstChar);
  it = OPERATORS.find(oneCharOp);
  if (it != OPERATORS.end()) {
    // 是合法的单字符运算符
    lexeme = oneCharOp;
    advance(); // 消费这个字符
    return Token(it->second, lexeme, startLine);
  }

  // 到这里说明既不是双字符运算符，也不是单字符运算符
  // 这种情况出现在：字符是 & 或 |，但不能单独构成运算符，也没有形成 && 或 ||

  if (firstChar == '&' || firstChar == '|') {
    // 单独的 & 或 | 在 SysY 中是非法的
    lexeme = oneCharOp;
    advance();
    ErrorHandler::reportLexicalError(startLine, "非法运算符: '" + lexeme +
                                                    "'，SysY 只支持 && 和 ||");
    return Token(TokenType::ERROR, lexeme, startLine);
  }

  // 其他情况（理论上不会发生，因为 isPotentialOperator 已经过滤了）
  lexeme = oneCharOp;
  advance();
  ErrorHandler::reportLexicalError(startLine, "非法字符: '" + lexeme + "'");
  return Token(TokenType::ERROR, lexeme, startLine);
}

Token Lexer::getNextToken() {
  while (!eofReached) {
    skipWhitespace();

    if (eofReached) {
      return Token(TokenType::END_OF_FILE, "", currentLine);
    }

    // 检查注释
    if (currentChar == '/' && (peek() == '/' || peek() == '*')) {
      skipComment();
      continue;
    }

    int startLine = currentLine;

    // 标识符或关键字
    if (isValidIdentifierChar(currentChar, true)) {
      return recognizeKeywordOrIdentifier();
    }

    // 数字
    if (std::isdigit(currentChar)) {
      return recognizeNumber();
    }

    // 运算符：需要检查所有可能成为运算符开头的字符
    // 包括那些不能单独作为运算符的字符，如 & 和 |
    if (isPotentialOperator(currentChar)) {
      return recognizeOperator();
    }

    // 分隔符
    auto sepIt = SEPARATORS.find(currentChar);
    if (sepIt != SEPARATORS.end()) {
      std::string lexeme(1, currentChar);
      Token token(sepIt->second, lexeme, startLine);
      advance();
      return token;
    }

    // 非法字符
    std::string illegalChar(1, currentChar);
    ErrorHandler::reportLexicalError(startLine,
                                     "非法字符: '" + illegalChar + "'");
    advance();
    return Token(TokenType::ERROR, illegalChar, startLine);
  }

  return Token(TokenType::END_OF_FILE, "", currentLine);
}

// 添加辅助函数
bool Lexer::isPotentialOperator(char c) {
  // 所有可能成为运算符（单字符或双字符）的起始字符
  static const std::string potentialOps = "+-*/%=!<>&|";
  return potentialOps.find(c) != std::string::npos;
}
