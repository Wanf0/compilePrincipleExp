#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

// 类型枚举
enum class VarType { INT, INT_ARRAY, VOID, UNKNOWN };

// 符号信息基类
class Symbol {
public:
  std::string name;
  int line;
  VarType type;
  bool isConst;
  bool isFunc;

  Symbol(const std::string &n, int ln, VarType t, bool isFunc = false,
         bool isConst = false)
      : name(n), line(ln), type(t), isConst(isConst), isFunc(isFunc) {}

  virtual ~Symbol() = default;
};

// 变量符号
class VarSymbol : public Symbol {
public:
  std::vector<int> dims; // 数组维度
  VarSymbol(const std::string &n, int ln, VarType t,
            const std::vector<int> &d = {}, bool isConst = false)
      : Symbol(n, ln, t, false, isConst), dims(d) {}
};

// 函数符号
class FuncSymbol : public Symbol {
public:
  std::vector<VarType> paramTypes;
  FuncSymbol(const std::string &n, int ln, VarType retType,
             const std::vector<VarType> &params = {})
      : Symbol(n, ln, retType, true), paramTypes(params) {}
};

// 作用域类
class Scope {
private:
  std::unordered_map<std::string, std::shared_ptr<Symbol>> symbols;

public:
  bool declare(const std::shared_ptr<Symbol> &sym);
  std::shared_ptr<Symbol> lookup(const std::string &name);
  bool exists(const std::string &name);
};

// 语义分析器
class SemanticAnalyzer {
private:
  std::stack<std::shared_ptr<Scope>> scopeStack;
  std::shared_ptr<FuncSymbol> currentFunc;
  bool inLoop; // 是否在循环体内
  bool hasMain;

  // 错误报告
  void reportError(int type, int line, const std::string &msg);

  // 作用域管理
  void enterScope();
  void exitScope();
  bool declareSymbol(const std::shared_ptr<Symbol> &sym);
  std::shared_ptr<Symbol> lookupSymbol(const std::string &name);

  // 类型检查辅助
  VarType getTypeFromAST(const std::unique_ptr<ASTNode> &node);
  bool typeCompatible(VarType t1, VarType t2);
  bool checkArrayAccess(const std::unique_ptr<ASTNode> &node);

  // AST 遍历
  void visitCompUnit(const std::unique_ptr<ASTNode> &node);
  void visitDecl(const std::unique_ptr<ASTNode> &node);
  void visitFuncDef(const std::unique_ptr<ASTNode> &node);
  void visitBlock(const std::unique_ptr<ASTNode> &node);
  void visitStmt(const std::unique_ptr<ASTNode> &node);
  void visitExp(const std::unique_ptr<ASTNode> &node);
  void visitLVal(const std::unique_ptr<ASTNode> &node);
  void visitFuncCall(const std::unique_ptr<ASTNode> &node);
  void visitAssignStmt(const std::unique_ptr<ASTNode> &node);
  void visitIfStmt(const std::unique_ptr<ASTNode> &node);
  void visitWhileStmt(const std::unique_ptr<ASTNode> &node);
  void visitReturnStmt(const std::unique_ptr<ASTNode> &node);

public:
  SemanticAnalyzer();
  void check(const std::unique_ptr<ASTNode> &ast);
  bool hasErrors();
};

#endif
