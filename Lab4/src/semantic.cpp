#include "../include/semantic.h"
#include <iostream>
#include <sstream>
#include <type_traits>

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
    : currentFunc(nullptr), inLoop(false), hasMain(false), errorCount(0) {
  enterScope(); // 全局作用域
}

void SemanticAnalyzer::reportError(int type, int line, const std::string &msg) {
  // 去重：同一类型在同一行同消息只报一次
  std::string key =
      std::to_string(type) + ":" + std::to_string(line) + ":" + msg;
  if (reportedErrors.find(key) != reportedErrors.end()) {
    return;
  }
  reportedErrors.insert(key);
  errorCount++;

  // 按实验要求格式输出
  std::cout << "Error type " << type << " at line " << line << " : " << msg
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

  // 如果当前作用域已有同名符号，则报告冲突
  if (scopeStack.top()->exists(sym->name)) {
    auto existing = scopeStack.top()->lookup(sym->name);
    if (existing) {
      if (existing->isFunc || sym->isFunc) {
        reportError(4, sym->line,
                    "function '" + sym->name + "' redefined or conflict");
      } else {
        reportError(2, sym->line, "variable '" + sym->name + "' redeclared");
      }
    } else {
      reportError(2, sym->line, "redeclaration '" + sym->name + "'");
    }
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

// ==================== 类型兼容性检查（实现补充） ====================

bool SemanticAnalyzer::typeCompatible(VarType t1, VarType t2) {
  // 精简规则：只有完全相同类型才视为兼容（INT 与 FLOAT 不互转）
  if (t1 == t2)
    return true;
  // INT 和 INT_ARRAY 在这里不兼容
  return false;
}

// ==================== AST 辅助函数 ====================

// 在子树中查找第一个 ID Terminal，并返回 lexeme 与行号
static bool findFirstID(const std::unique_ptr<ASTNode> &node,
                        std::string &outName, int &outLine) {
  if (!node)
    return false;
  const TerminalNode *t = dynamic_cast<const TerminalNode *>(node.get());
  if (t && t->getName() == "ID") {
    outName = t->getLexeme();
    outLine = t->getLine();
    return true;
  }
  for (const auto &c : node->getChildren()) {
    if (findFirstID(c, outName, outLine))
      return true;
  }
  return false;
}

// 从 BType 子节点推断变量/函数返回类型
static VarType getVarTypeFromBType(const std::unique_ptr<ASTNode> &btypeNode) {
  if (!btypeNode)
    return VarType::UNKNOWN;
  if (btypeNode->getChildren().empty())
    return VarType::UNKNOWN;
  const ASTNode *maybeTok = btypeNode->getChildren()[0].get();
  const TerminalNode *t = dynamic_cast<const TerminalNode *>(maybeTok);
  if (!t)
    return VarType::UNKNOWN;
  if (t->getName() == "INTTK")
    return VarType::INT;
  if (t->getName() == "FLOATTK")
    return VarType::FLOAT;
  if (t->getName() == "VOIDTK")
    return VarType::VOID;
  return VarType::UNKNOWN;
}

// 递归计算表达式类型，如果发生运算类型不匹配，会报告 Error type 11
VarType SemanticAnalyzer::getTypeFromAST(const std::unique_ptr<ASTNode> &node) {
  if (!node)
    return VarType::UNKNOWN;

  // 终结符处理
  const TerminalNode *t = dynamic_cast<const TerminalNode *>(node.get());
  if (t) {
    const std::string &n = t->getName();
    if (n == "INTCON") {
      return VarType::INT;
    } else if (n == "ID") {
      auto sym = lookupSymbol(t->getLexeme());
      if (!sym) {
        // 未声明的变量已在 visitLVal 中报告；返回 UNKNOWN 以避免连锁错误
        return VarType::UNKNOWN;
      }
      // 若标识符是函数名而作为表达式出现，若函数有返回类型则返回
      return sym->type;
    } else {
      // 运算符等终结符不单独作为类型
      return VarType::UNKNOWN;
    }
  }

  // 非终结符（按节点名称处理常见表达式类型）
  const std::string nm = node->getName();

  // 函数调用：返回函数声明的类型（如果函数未定义，visitFuncCall 已报告）
  if (nm == "FuncCall") {
    std::string fname;
    int line = node->getLine();
    if (findFirstID(node, fname, line)) {
      auto sym = lookupSymbol(fname);
      if (sym && sym->isFunc) {
        return sym->type;
      } else {
        return VarType::UNKNOWN;
      }
    }
    return VarType::UNKNOWN;
  }

  // 一元运算：只检查子节点
  if (nm == "UnaryExp" || nm == "UnaryOp") {
    if (!node->getChildren().empty()) {
      VarType sub = getTypeFromAST(node->getChildren().back());
      return sub;
    }
    return VarType::UNKNOWN;
  }

  // 二元表达式：BinaryExp / RelExp / EqExp / LAndExp / LOrExp / MulExp / AddExp
  // 约定：二元节点的结构通常为 [left, opTerminal, right] 或更复杂的嵌套形式
  if (nm == "BinaryExp" || nm == "RelExp" || nm == "EqExp" || nm == "LAndExp" ||
      nm == "LOrExp" || nm == "AddExp" || nm == "MulExp" || nm == "RelExp" ||
      nm == "EqExp") {
    const auto &children = node->getChildren();
    if (children.size() >= 3) {
      VarType left = getTypeFromAST(children[0]);
      // operator may be children[1] (TerminalNode)
      const ASTNode *opNode = children[1].get();
      const TerminalNode *opT = dynamic_cast<const TerminalNode *>(opNode);
      VarType right = getTypeFromAST(children[2]);

      // 如果任一子表达式类型未知，难以进一步判断
      if (left == VarType::UNKNOWN || right == VarType::UNKNOWN) {
        return VarType::UNKNOWN;
      }

      // 对于算术运算 + - * / % 要求两个操作数为 INT（本实验中视 INT 与 FLOAT
      // 不可混合）
      std::string opname = opT ? opT->getName() : "";
      bool isArithmetic =
          (opname == "PLUS" || opname == "MINUS" || opname == "MULTIPLY" ||
           opname == "DIVIDE" || opname == "MOD");
      bool isRelational = (opname == "LT" || opname == "LTE" ||
                           opname == "GT" || opname == "GTE");
      bool isEquality = (opname == "EQ" || opname == "NEQ");
      bool isLogical = (opname == "AND" || opname == "OR" || opname == "NOT");

      if (isArithmetic || isRelational || isEquality || isLogical) {
        // 对这些操作符，要求两边类型相同并且为 INT（当前实验规则）
        if (left != right || left != VarType::INT) {
          int errLine = (opT ? opT->getLine() : node->getLine());
          reportError(11, errLine, "type mismatched for operands");
          return VarType::UNKNOWN;
        }
        // 算术运算和关系运算结果为 INT（简化）
        return VarType::INT;
      }
    }
  }

  // 普通节点：尝试递归返回第一个非空子节点类型
  for (const auto &c : node->getChildren()) {
    VarType tsub = getTypeFromAST(c);
    if (tsub != VarType::UNKNOWN)
      return tsub;
  }

  return VarType::UNKNOWN;
}

// ==================== AST 遍历方法 ====================

void SemanticAnalyzer::visitCompUnit(const std::unique_ptr<ASTNode> &node) {
  if (!node)
    return;

  hasMain = false;

  // 遍历所有声明和函数定义
  for (const auto &child : node->getChildren()) {
    std::string name = child->getName();
    if (name == "FuncDef") {
      visitFuncDef(child);
    } else if (name == "ConstDecl" || name == "VarDecl") {
      visitDecl(child);
    } else {
      // 其他节点忽略
    }
  }

  // 检查 main 函数是否存在（如果不存在，报告错误 type 3）
  if (!hasMain) {
    reportError(3, 1, "missing main function");
  }
}

void SemanticAnalyzer::visitDecl(const std::unique_ptr<ASTNode> &node) {
  // VarDecl 的子节点结构：BType, VarDef...
  if (!node)
    return;
  const auto &children = node->getChildren();
  if (children.empty())
    return;

  // 第一个子节点应该是 BType
  VarType declaredType = VarType::UNKNOWN;
  if (children[0]->getName() == "BType") {
    declaredType = getVarTypeFromBType(children[0]);
  }

  // 在 VarDecl 中找到所有 ID 并声明变量（支持多个逗号分隔的 VarDef）
  for (size_t i = 1; i < children.size(); ++i) {
    std::string varName;
    int line = children[i]->getLine();
    // 若节点本身是 Terminal ID
    const TerminalNode *tn =
        dynamic_cast<const TerminalNode *>(children[i].get());
    if (tn && tn->getName() == "ID") {
      varName = tn->getLexeme();
      line = tn->getLine();
    } else {
      // 否则在子树里查找第一个ID
      findFirstID(children[i], varName, line);
    }
    if (varName.empty())
      continue;
    auto varSym = std::make_shared<VarSymbol>(varName, line, declaredType);
    declareSymbol(varSym);
  }
}

void SemanticAnalyzer::visitFuncDef(const std::unique_ptr<ASTNode> &node) {
  if (!node)
    return;

  const auto &children = node->getChildren();
  if (children.size() < 2)
    return;

  // 获取函数名（在第一个 ID terminal 节点）
  std::string funcName;
  int nameLine = node->getLine();
  if (!findFirstID(node, funcName, nameLine)) {
    return;
  }

  // 标记 main
  if (funcName == "main") {
    hasMain = true;
  }

  // 检查函数是否已定义（任何作用域）
  auto existing = lookupSymbol(funcName);
  if (existing && existing->isFunc) {
    reportError(4, nameLine, "function '" + funcName + "' redefined");
    return;
  }

  // 获取返回类型（从第一个子节点 FuncType 推断）
  VarType retType = VarType::INT;
  if (!children.empty() && children[0]->getName() == "FuncType") {
    retType = getVarTypeFromBType(children[0]);
    if (retType == VarType::UNKNOWN)
      retType = VarType::INT;
  }

  // 创建函数符号并在当前（全局）作用域声明
  auto funcSym = std::make_shared<FuncSymbol>(funcName, nameLine, retType);
  if (!declareSymbol(funcSym)) {
    return;
  }

  // 设置当前函数
  currentFunc = funcSym;

  // 进入函数体作用域
  enterScope();

  // 遍历函数体（寻找 Block 节点）
  for (const auto &c : children) {
    if (c->getName() == "Block") {
      visitBlock(c);
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
    const std::string &nm = child->getName();
    if (nm == "VarDecl" || nm == "ConstDecl") {
      visitDecl(child);
    } else if (nm == "Stmt" || nm.find("Stmt") != std::string::npos ||
               nm == "Block") {
      visitStmt(child);
    } else {
      // 递归其它（防御性处理）
      visitStmt(child);
    }
  }

  exitScope();
}

void SemanticAnalyzer::visitStmt(const std::unique_ptr<ASTNode> &node) {
  if (!node)
    return;

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
      reportError(12, node->getLine(), "break not in loop");
    }
  } else if (stmtType == "ContinueStmt") {
    if (!inLoop) {
      reportError(13, node->getLine(), "continue not in loop");
    }
  } else if (stmtType == "ExpStmt") {
    visitExp(node);
  } else {
    // 一般节点递归检查其子节点
    visitExp(node);
  }
}

void SemanticAnalyzer::visitExp(const std::unique_ptr<ASTNode> &node) {
  if (!node)
    return;

  // 递归处理子节点并在遇到表达式二元节点时求值以触发类型检查
  for (const auto &child : node->getChildren()) {
    const std::string &nm = child->getName();
    if (nm == "LVal") {
      visitLVal(child);
    } else if (nm == "FuncCall") {
      visitFuncCall(child);
    } else if (nm == "BinaryExp" || nm == "RelExp" || nm == "EqExp" ||
               nm == "LAndExp" || nm == "LOrExp") {
      // 对二元表达式进行类型推导（会在不匹配时报告 Error type 11）
      (void)getTypeFromAST(child);
      // 继续递归内部以检查更深层的 LVal/FuncCall
      visitExp(child);
    } else {
      visitExp(child);
    }
  }
}

void SemanticAnalyzer::visitLVal(const std::unique_ptr<ASTNode> &node) {
  if (!node)
    return;

  std::string varName;
  int line = node->getLine();
  if (!findFirstID(node, varName, line)) {
    return;
  }

  // 查找符号
  auto symbol = lookupSymbol(varName);
  if (!symbol) {
    reportError(1, line, "undefined variable \"" + varName + "\"");
    return;
  }

  // 检查是否是函数被当作变量使用
  if (symbol->isFunc) {
    reportError(6, line, "function name \"" + varName + "\" used as variable");
    return;
  }

  // 如果有数组下标访问，简单检查符号类型是否为数组
  const auto &children = node->getChildren();
  if (children.size() > 1) {
    if (symbol->type != VarType::INT_ARRAY) {
      reportError(8, line, "subscripted non-array \"" + varName + "\"");
      return;
    }
    // TODO: 对下标表达式类型进行检查（是否为整型）
  }
}

void SemanticAnalyzer::visitFuncCall(const std::unique_ptr<ASTNode> &node) {
  if (!node)
    return;

  std::string funcName;
  int line = node->getLine();
  if (!findFirstID(node, funcName, line)) {
    return;
  }

  // 查找函数符号（在所有作用域中查找）
  auto symbol = lookupSymbol(funcName);
  if (!symbol) {
    // 未定义的函数 -> Error type 3
    reportError(3, line, "undefined function \"" + funcName + "\"");
    return;
  }

  // 如果存在但不是函数 -> 把变量当作函数调用 -> Error type 5
  if (!symbol->isFunc) {
    reportError(5, line, "variable \"" + funcName + "\" used as function");
    return;
  }

  // TODO: 参数数量/类型检查 (Error type 9)
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
      // 触发类型推导，若类型不匹配可在此处扩展检查
      VarType rhs = getTypeFromAST(children[1]);
      // 若左是ID可进一步检查赋值类型一致性
      std::string leftName;
      int leftLine = children[0]->getLine();
      if (findFirstID(children[0], leftName, leftLine)) {
        auto sym = lookupSymbol(leftName);
        if (sym && !sym->isFunc && rhs != VarType::UNKNOWN &&
            sym->type != rhs) {
          reportError(11, node->getLine(), "type mismatched for operands");
        }
      }
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
    reportError(10, node->getLine(), "return not in function");
    return;
  }

  const auto &children = node->getChildren();
  if (children.empty()) {
    // 无返回值
    if (currentFunc->type != VarType::VOID) {
      reportError(10, node->getLine(),
                  "non-void function missing return value");
    }
  } else {
    // 有返回值
    if (currentFunc->type == VarType::VOID) {
      reportError(10, node->getLine(), "void function returning a value");
    } else {
      VarType expType = getTypeFromAST(children[0]);
      if (expType == VarType::UNKNOWN ||
          !typeCompatible(currentFunc->type, expType)) {
        reportError(10, node->getLine(), "return type mismatch");
      }
    }
  }
}

void SemanticAnalyzer::check(const std::unique_ptr<ASTNode> &ast) {
  if (!ast)
    return;

  if (ast->getName() == "CompUnit") {
    visitCompUnit(ast);
  } else {
    // 如果传入的不是 CompUnit 就尝试从根节点递归查找
    visitCompUnit(ast);
  }
}

bool SemanticAnalyzer::hasErrors() { return errorCount > 0; }
