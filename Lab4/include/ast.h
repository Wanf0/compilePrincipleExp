#ifndef AST_H
#define AST_H

#include "token.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// AST节点类型枚举
enum class ASTNodeType {
  // 编译单元相关
  COMP_UNIT,
  DECL,
  CONST_DECL,
  VAR_DECL,
  FUNC_DEF,

  // 类型相关
  TYPE,

  // 表达式相关
  EXPR,
  BINARY_EXPR,
  UNARY_EXPR,
  LVAL,
  PRIMARY_EXPR,
  NUMBER,

  // 语句相关
  STMT,
  BLOCK,
  IF_STMT,
  WHILE_STMT,
  RETURN_STMT,
  ASSIGN_STMT,
  EXPR_STMT,
  BREAK_STMT,
  CONTINUE_STMT,

  // 函数相关
  FUNC_PARAM,
  FUNC_CALL,
  FUNC_TYPE,

  // 初始化相关
  INIT_VAL,

  // 特殊节点
  EMPTY
};

// AST节点基类
class ASTNode {
protected:
  ASTNodeType nodeType;
  int line;
  std::string name;
  std::vector<std::unique_ptr<ASTNode>> children;

public:
  ASTNode(ASTNodeType type, int line = -1, const std::string &name = "")
      : nodeType(type), line(line), name(name) {}

  virtual ~ASTNode() = default;

  void addChild(std::unique_ptr<ASTNode> child) {
    children.push_back(std::move(child));
  }

  const std::vector<std::unique_ptr<ASTNode>> &getChildren() const {
    return children;
  }

  ASTNodeType getType() const { return nodeType; }
  int getLine() const { return line; }
  const std::string &getName() const { return name; }

  // 输出节点信息
  virtual std::string toString() const = 0;

  // 先序遍历打印
  void printPreOrder(int depth = 0) const {
    // 输出缩进
    for (int i = 0; i < depth; i++) {
      std::cout << "  ";
    }

    // 输出节点信息
    std::cout << toString() << std::endl;

    // 递归输出子节点
    for (const auto &child : children) {
      child->printPreOrder(depth + 1);
    }
  }
};

// 语法单元节点（非终结符）
class NonTerminalNode : public ASTNode {
private:
  bool isEmpty;

public:
  NonTerminalNode(const std::string &name, int line = -1, bool isEmpty = false)
      : ASTNode(ASTNodeType::COMP_UNIT, line, name), isEmpty(isEmpty) {}
  virtual std::string getLexeme() const { return ""; }
  std::string toString() const override {
    if (isEmpty) {
      return name; // ε产生式，只打印名称
    } else {
      return name + " (" + std::to_string(line) + ")";
    }
  }

  bool getIsEmpty() const { return isEmpty; }
  void setIsEmpty(bool empty) { isEmpty = empty; }
};

// 词法单元节点（终结符）
class TerminalNode : public ASTNode {
private:
  std::string lexeme;
  int value;

public:
  TerminalNode(const std::string &tokenName, const std::string &lexeme,
               int line = -1, int value = 0)
      : ASTNode(ASTNodeType::EXPR, line, tokenName), lexeme(lexeme),
        value(value) {}
  // std::string getLexeme() const override { return lexeme; }
  std::string toString() const override {
    // 需要输出额外信息的Token类型
    if (name == "ID") {
      return name + " : " + lexeme;
    } else if (name == "INTCON") {
      return name + " : " + std::to_string(value);
    } else {
      return name;
    }
  }

  const std::string &getLexeme() const { return lexeme; }
  int getValue() const { return value; }
};

#endif
