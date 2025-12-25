#include "../include/semantic.h"
#include <iostream>
#include <sstream>

// ==================== Scope 实现 ====================

bool Scope::declare(const std::shared_ptr<Symbol> &sym) {
  if (exists(sym->name)) {
    return false;
  }
  symbols[sym->name] = sym;
  return true;
}

std::shared_ptr<Symbol> Scope::lookup(const std::string &name) {
  auto it = symbols.find(name);
  if (it != symbols.end()) {
    return it->second;
  }
  return nullptr;
}

bool Scope::exists(const std::string &name) {
  return symbols.find(name) != symbols.end();
}

// ==================== SemanticAnalyzer 实现 ====================

SemanticAnalyzer::SemanticAnalyzer()
    : currentFunc(nullptr), inLoop(false), hasMain(false) {
  enterScope(); // 全局作用域
}

void SemanticAnalyzer::reportError(int type, int line, const std::string &msg) {
  std::cout << "Error type " << type << " at line " << line << ": " << msg
            << std::endl;
}

void SemanticAnalyzer::enterScope() {
  scopeStack.push(std::make_shared<Scope>());
}

void SemanticAnalyzer::exitScope() {
  if (!scopeStack.empty()) {
    scopeStack.pop();
  }
}

bool SemanticAnalyzer::declareSymbol(const std::shared_ptr<Symbol> &sym) {
  if (scopeStack.empty())
    return false;

  // 检查当前作用域是否已声明
  if (scopeStack.top()->exists(sym->name)) {
    reportError(2, sym->line, "变量 '" + sym->name + "' 重复声明");
    return false;
  }

  scopeStack.top()->declare(sym);
  return true;
}

std::shared_ptr<Symbol>
SemanticAnalyzer::lookupSymbol(const std::string &name) {
  // 从内层作用域向外层查找
  std::stack<std::shared_ptr<Scope>> tempStack = scopeStack;

  while (!tempStack.empty()) {
    auto scope = tempStack.top();
    auto symbol = scope->lookup(name);
    if (symbol) {
      return symbol;
    }
    tempStack.pop();
  }

  return nullptr;
}

VarType SemanticAnalyzer::getTypeFromAST(const std::unique_ptr<ASTNode> &node) {
  // 简化实现：假设节点类型信息在名字中
  if (!node)
    return VarType::UNKNOWN;

  // 这里需要根据实际的AST结构来解析类型
  // 暂时返回默认值
  return VarType::INT;
}

bool SemanticAnalyzer::typeCompatible(VarType t1, VarType t2) {
  // SysY 只有int和void类型，数组需要特殊处理
  if (t1 == VarType::INT && t2 == VarType::INT)
    return true;
  if (t1 == VarType::VOID && t2 == VarType::VOID)
    return true;
  // 暂时简化处理
  return false;
}

bool SemanticAnalyzer::checkArrayAccess(const std::unique_ptr<ASTNode> &node) {
  // TODO: 实现数组访问检查
  return true;
}

// ==================== AST 遍历方法 ====================

void SemanticAnalyzer::visitCompUnit(const std::unique_ptr<ASTNode> &node) {
  if (!node)
    return;

  hasMain = false;

  // 遍历所有声明和函数定义
  for (const auto &child : node->getChildren()) {
    if (child->getName() == "FuncDef") {
      visitFuncDef(child);
    } else if (child->getName() == "ConstDecl" ||
               child->getName() == "VarDecl") {
      visitDecl(child);
    }
  }

  // 检查main函数
  if (!hasMain) {
    reportError(3, 1, "缺少 main 函数");
  }
}

void SemanticAnalyzer::visitDecl(const std::unique_ptr<ASTNode> &node) {
  // TODO: 实现变量声明检查
  // 简化实现：假设第一个子节点是类型，第二个是标识符
  const auto &children = node->getChildren();
  if (children.size() >= 2) {
    std::string varName = ""; // 需要从TerminalNode获取
                              // 尝试从TerminalNode获取变量名
    if (children.size() > 1) {
      // 这里假设第二个子节点是TerminalNode，存储变量名
      // 实际实现需要根据AST结构调整
      auto varNode = children[1].get();
      varName = varNode->getName(); // 需要ASTNode提供getLexeme()方法
    }
    int line = children[1]->getLine();

    // 创建变量符号
    auto varSym = std::make_shared<VarSymbol>(varName, line, VarType::INT);

    // 声明符号
    if (!declareSymbol(varSym)) {
      // 错误已经在declareSymbol中报告
    }
  }
  (void)node; // 消除未使用变量警告
}

void SemanticAnalyzer::visitFuncDef(const std::unique_ptr<ASTNode> &node) {
  if (!node)
    return;

  const auto &children = node->getChildren();
  if (children.size() < 3)
    return;

  // 获取函数名（假设第二个子节点是函数名）
  std::string funcName = ""; // 需要从TerminalNode获取
  int line = children[1]->getLine();

  // 尝试从节点获取函数名
  // 这里需要根据实际的AST结构来获取
  // 假设ASTNode有方法可以获取原始标识符文本
  if (children.size() > 1) {
    auto funcNameNode = children[1].get();
    funcName = funcNameNode->getName(); // 需要ASTNode提供getLexeme()方法
  }
  // 检查是否为main函数
  if (funcName == "main") {
    hasMain = true;
    // TODO: 检查main函数签名
  }

  // 检查函数是否已定义
  auto existing = lookupSymbol(funcName);
  if (existing && existing->isFunc) {
    reportError(4, line, "函数 '" + funcName + "' 重复定义");
    return;
  }

  // 获取返回类型
  VarType retType = VarType::INT; // 简化处理
  // TODO: 从第一个子节点解析返回类型

  // 创建函数符号
  auto funcSym = std::make_shared<FuncSymbol>(funcName, line, retType);

  // 在全局作用域声明函数
  if (!declareSymbol(funcSym)) {
    return;
  }

  // 设置当前函数
  currentFunc = funcSym;

  // 进入函数体作用域
  enterScope();

  // 遍历函数体
  for (size_t i = 2; i < children.size(); i++) {
    if (children[i]->getName() == "Block") {
      visitBlock(children[i]);
    }
  }

  // 退出函数体作用域
  exitScope();

  // 清除当前函数
  currentFunc = nullptr;
}

void SemanticAnalyzer::visitBlock(const std::unique_ptr<ASTNode> &node) {
  enterScope();

  for (const auto &child : node->getChildren()) {
    if (child->getName() == "Decl") {
      visitDecl(child);
    } else if (child->getName() == "Stmt" ||
               child->getName().find("Stmt") != std::string::npos) {
      visitStmt(child);
    }
  }

  exitScope();
}

void SemanticAnalyzer::visitStmt(const std::unique_ptr<ASTNode> &node) {
  std::string stmtType = node->getName();

  if (stmtType == "AssignStmt") {
    visitAssignStmt(node);
  } else if (stmtType == "IfStmt") {
    visitIfStmt(node);
  } else if (stmtType == "WhileStmt") {
    visitWhileStmt(node);
  } else if (stmtType == "ReturnStmt") {
    visitReturnStmt(node);
  } else if (stmtType == "BreakStmt") {
    if (!inLoop) {
      reportError(12, node->getLine(), "break 不在循环体内");
    }
  } else if (stmtType == "ContinueStmt") {
    if (!inLoop) {
      reportError(13, node->getLine(), "continue 不在循环体内");
    }
  } else if (stmtType == "ExpStmt") {
    visitExp(node);
  }
}

void SemanticAnalyzer::visitExp(const std::unique_ptr<ASTNode> &node) {
  // 检查表达式中的变量使用
  for (const auto &child : node->getChildren()) {
    if (child->getName() == "LVal") {
      visitLVal(child);
    } else if (child->getName() == "FuncCall") {
      visitFuncCall(child);
    } else if (child->getName() == "BinaryExp") {
      visitExp(child); // 递归检查
    }
  }
}

void SemanticAnalyzer::visitLVal(const std::unique_ptr<ASTNode> &node) {
  const auto &children = node->getChildren();
  if (children.empty())
    return;

  // 第一个子节点应该是标识符
  std::string varName = ""; // 需要从TerminalNode获取
  int line = children[0]->getLine();
  // 获取变量名 - 假设第一个子节点是标识符

  // 尝试从节点获取变量名
  // 这里需要根据实际的AST结构来获取
  if (!children.empty()) {
    auto varNameNode = children[0].get();
    varName = varNameNode->getName(); // 需要ASTNode提供getLexeme()方法
  }

  // 如果变量名为空，不进行后续检查
  if (varName.empty()) {
    return;
  }
  // 查找符号
  auto symbol = lookupSymbol(varName);
  if (!symbol) {
    reportError(1, line, "未声明的变量 '" + varName + "'");
    return;
  }

  // 检查是否是函数被当作变量使用
  if (symbol->isFunc) {
    reportError(6, line, "函数名 '" + varName + "' 不能作为变量使用");
    return;
  }

  // 检查数组访问
  if (children.size() > 1) {
    // 有数组下标访问
    if (symbol->type != VarType::INT_ARRAY) {
      reportError(8, line, "对非数组变量 '" + varName + "' 使用数组访问");
      return;
    }

    // 检查下标是否为整型
    // TODO: 实现下标类型检查
  }
}

void SemanticAnalyzer::visitFuncCall(const std::unique_ptr<ASTNode> &node) {
  const auto &children = node->getChildren();
  if (children.empty())
    return;

  std::string funcName = ""; // 需要从TerminalNode获取
  int line = children[0]->getLine();
  // 尝试从节点获取函数名
  if (!children.empty()) {
    auto funcNameNode = children[0].get();
    funcName = funcNameNode->getName(); // 需要ASTNode提供getLexeme()方法
  }

  // 如果函数名为空，不进行后续检查
  if (funcName.empty()) {
    return;
  }
  // 查找函数符号
  auto symbol = lookupSymbol(funcName);
  if (!symbol) {
    reportError(3, line, "未定义的函数 '" + funcName + "'");
    return;
  }

  // 检查是否是变量被当作函数调用
  if (!symbol->isFunc) {
    reportError(5, line, "变量 '" + funcName + "' 不能被作为函数调用");
    return;
  }

  // 类型转换为FuncSymbol
  auto funcSymbol = std::dynamic_pointer_cast<FuncSymbol>(symbol);
  if (!funcSymbol)
    return;

  // TODO: 检查参数数量和类型
  // 这里需要统计实际参数数量和类型，与funcSymbol->paramTypes比较
}

void SemanticAnalyzer::visitAssignStmt(const std::unique_ptr<ASTNode> &node) {
  const auto &children = node->getChildren();
  if (children.size() >= 2) {
    // 检查左值
    if (children[0]->getName() == "LVal") {
      visitLVal(children[0]);
    }

    // 检查右值表达式
    if (children.size() > 1) {
      visitExp(children[1]);
    }
  }
}

void SemanticAnalyzer::visitIfStmt(const std::unique_ptr<ASTNode> &node) {
  const auto &children = node->getChildren();
  if (!children.empty()) {
    // 检查条件表达式
    visitExp(children[0]);

    // 检查then语句
    if (children.size() > 1) {
      visitStmt(children[1]);
    }

    // 检查else语句
    if (children.size() > 2) {
      visitStmt(children[2]);
    }
  }
}

void SemanticAnalyzer::visitWhileStmt(const std::unique_ptr<ASTNode> &node) {
  const auto &children = node->getChildren();
  if (children.size() >= 2) {
    // 检查条件表达式
    visitExp(children[0]);

    // 设置循环标志
    bool oldInLoop = inLoop;
    inLoop = true;

    // 检查循环体
    visitStmt(children[1]);

    // 恢复循环标志
    inLoop = oldInLoop;
  }
}

void SemanticAnalyzer::visitReturnStmt(const std::unique_ptr<ASTNode> &node) {
  if (!currentFunc) {
    reportError(10, node->getLine(), "return 语句不在函数内");
    return;
  }

  const auto &children = node->getChildren();
  if (children.empty()) {
    // 无返回值
    if (currentFunc->type != VarType::VOID) {
      reportError(10, node->getLine(),
                  "无返回值的 return 语句在非 void 函数中");
    }
  } else {
    // 有返回值
    if (currentFunc->type == VarType::VOID) {
      reportError(10, node->getLine(), "有返回值的 return 语句在 void 函数中");
    } else {
      // TODO: 检查返回值类型是否匹配
      VarType expType = getTypeFromAST(children[0]);
      if (!typeCompatible(currentFunc->type, expType)) {
        reportError(10, node->getLine(), "return 类型与函数声明不一致");
      }
    }
  }
}

void SemanticAnalyzer::check(const std::unique_ptr<ASTNode> &ast) {
  if (!ast)
    return;

  if (ast->getName() == "CompUnit") {
    visitCompUnit(ast);
  }
}

bool SemanticAnalyzer::hasErrors() {
  // TODO: 实现错误计数
  return false;
}
