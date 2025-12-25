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

// ==================== 类型兼容性检查 ====================

bool SemanticAnalyzer::typeCompatible(VarType t1, VarType t2) {
  // 精简规则：只有完全相同类型才视为兼容（INT 与 FLOAT 不互转）
  if (t1 == t2)
    return true;
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

// ==================== 类型推导（表达式） ====================

VarType SemanticAnalyzer::getTypeFromAST(const std::unique_ptr<ASTNode> &node) {
  if (!node)
    return VarType::UNKNOWN;

  // 终结符处理
  const TerminalNode *t = dynamic_cast<const TerminalNode *>(node.get());
  if (t) {
    const std::string &n = t->getName();
    if (n == "INTCON") {
      return VarType::INT;
    } else if (n == "FLOATCON") {
      return VarType::FLOAT;
    } else if (n == "ID") {
      auto sym = lookupSymbol(t->getLexeme());
      if (!sym) {
        // 未声明的变量已在 visitLVal 中报告；返回 UNKNOWN 以避免连锁错误
        return VarType::UNKNOWN;
      }
      return sym->type;
    } else {
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

  // 一元运算：只检查子节点（取最后一个子节点类型）
  if (nm == "UnaryExp" || nm == "UnaryOp") {
    if (!node->getChildren().empty()) {
      VarType sub = getTypeFromAST(node->getChildren().back());
      return sub;
    }
    return VarType::UNKNOWN;
  }

  // 二元表达式：一般模式 [left, op, right]
  if (nm == "BinaryExp" || nm == "RelExp" || nm == "EqExp" || nm == "LAndExp" ||
      nm == "LOrExp" || nm == "AddExp" || nm == "MulExp") {
    const auto &children = node->getChildren();
    if (children.size() >= 3) {
      VarType left = getTypeFromAST(children[0]);
      const ASTNode *opNode = children[1].get();
      const TerminalNode *opT = dynamic_cast<const TerminalNode *>(opNode);
      VarType right = getTypeFromAST(children[2]);

      // 若任一子表达式未知则无法判定类型
      if (left == VarType::UNKNOWN || right == VarType::UNKNOWN) {
        return VarType::UNKNOWN;
      }

      std::string opname = opT ? opT->getName() : "";
      bool isArithmetic =
          (opname == "PLUS" || opname == "MINUS" || opname == "MULTIPLY" ||
           opname == "DIVIDE" || opname == "MOD");
      bool isRelational = (opname == "LT" || opname == "LTE" ||
                           opname == "GT" || opname == "GTE");
      bool isEquality = (opname == "EQ" || opname == "NEQ");
      bool isLogical = (opname == "AND" || opname == "OR" || opname == "NOT");

      if (isArithmetic || isRelational || isEquality || isLogical) {
        // 本实验简化规则：要求两边类型相同且为 INT（不支持隐式转换）
        if (left != right || left != VarType::INT) {
          int errLine = (opT ? opT->getLine() : node->getLine());
          reportError(11, errLine, "type mismatched for operands");
          return VarType::UNKNOWN;
        }
        // 结果类型简化为 INT
        return VarType::INT;
      }
    }
  }

  // 递归尝试从子节点推断类型（取第一个有效子类型）
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

  // 检查 main 函数是否存在（若不存在，报告 error type 3）
  if (!hasMain) {
    reportError(3, 1, "missing main function");
  }
}

void SemanticAnalyzer::visitDecl(const std::unique_ptr<ASTNode> &node) {
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

  // 从第二个子节点开始，每个 VarDef 节点代表一个变量定义
  for (size_t idx = 1; idx < children.size(); ++idx) {
    const auto &child = children[idx];
    if (child->getName() == "VarDef") {
      // VarDef 内结构： ID, {Index...}, [InitVal]
      const auto &vchildren = child->getChildren();
      if (vchildren.empty())
        continue;
      // ID 应该是第一个子节点
      const TerminalNode *idn =
          dynamic_cast<const TerminalNode *>(vchildren[0].get());
      std::string varName;
      int varLine = child->getLine();
      if (idn && idn->getName() == "ID") {
        varName = idn->getLexeme();
        varLine = idn->getLine();
      } else {
        // 防御性查找
        findFirstID(child, varName, varLine);
      }
      if (varName.empty())
        continue;

      // 判断是否有 Index 节点以标记数组
      bool isArray = false;
      for (size_t k = 1; k < vchildren.size(); ++k) {
        if (vchildren[k]->getName() == "Index") {
          isArray = true;
          break;
        }
      }

      VarType symType = declaredType;
      if (isArray) {
        symType = VarType::INT_ARRAY; // 本实验仅支持 int 数组元素类型
      }

      auto varSym = std::make_shared<VarSymbol>(varName, varLine, symType);
      declareSymbol(varSym);
    } else {
      // 兼容之前老结构：若不是 VarDef，尝试按旧逻辑处理（兼容性）
      std::string varName;
      int varLine = children[idx]->getLine();
      if (!findFirstID(children[idx], varName, varLine))
        continue;
      VarType symType = declaredType;
      auto varSym = std::make_shared<VarSymbol>(varName, varLine, symType);
      declareSymbol(varSym);
    }
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

  // 提取形式参数信息并填充 funcSym->paramTypes（若有）
  for (const auto &c : children) {
    if (c->getName() == "FuncFParams") {
      for (const auto &fp : c->getChildren()) {
        // fp: FuncFParam -> [BType, ID]
        VarType ptype = VarType::UNKNOWN;
        if (!fp->getChildren().empty()) {
          if (fp->getChildren()[0]->getName() == "BType")
            ptype = getVarTypeFromBType(fp->getChildren()[0]);
        }
        if (ptype == VarType::UNKNOWN)
          ptype = VarType::INT;
        funcSym->paramTypes.push_back(ptype);
      }
    }
  }

  // 设置当前函数
  currentFunc = funcSym;

  // 进入函数体作用域并把形参作为局部变量声明
  enterScope();
  for (const auto &c : children) {
    if (c->getName() == "FuncFParams") {
      size_t pi = 0;
      for (const auto &fp : c->getChildren()) {
        std::string pname;
        int pline = fp->getLine();
        if (findFirstID(fp, pname, pline)) {
          VarType ptype = VarType::INT;
          if (pi < funcSym->paramTypes.size())
            ptype = funcSym->paramTypes[pi];
          auto paramSym = std::make_shared<VarSymbol>(pname, pline, ptype);
          declareSymbol(paramSym);
          ++pi;
        }
      }
    }
  }

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

  const auto &children = node->getChildren();
  if (children.empty())
    return;

  // 第一个子节点应该是标识符
  std::string varName;
  int line = children[0]->getLine();
  const TerminalNode *idNode =
      dynamic_cast<const TerminalNode *>(children[0].get());
  if (idNode && idNode->getName() == "ID") {
    varName = idNode->getLexeme();
    line = idNode->getLine();
  } else {
    if (!findFirstID(node, varName, line))
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

  // 如果有数组下标访问，先检查下标类型（优先报 type 7），再检查是否为数组（type
  // 8）
  if (children.size() > 1) {
    // children[1..] 是下标表达式（在 parseLVal 中构造）
    for (size_t idx = 1; idx < children.size(); ++idx) {
      VarType idxType = getTypeFromAST(children[idx]);
      // 发现非整数下标 -> 报 type 7
      if (idxType != VarType::INT) {
        // 若是浮点字面量，尽量打印原始文本
        const TerminalNode *tn =
            dynamic_cast<const TerminalNode *>(children[idx].get());
        if (tn && tn->getName() == "FLOATCON") {
          reportError(7, tn->getLine(),
                      "\"" + tn->getLexeme() + "\" is not an integer");
        } else if (tn && tn->getName() == "ID") {
          auto s = lookupSymbol(tn->getLexeme());
          if (s && s->type == VarType::FLOAT) {
            reportError(7, tn->getLine(),
                        "\"" + tn->getLexeme() + "\" is not an integer");
          } else {
            reportError(7, children[idx]->getLine(),
                        "array subscript is not integer");
          }
        } else {
          reportError(7, children[idx]->getLine(),
                      "array subscript is not integer");
        }
        // 继续检查下一个下标（reportError 会去重）
      }
    }

    // 检查是否为数组类型
    if (symbol->type != VarType::INT_ARRAY) {
      // 只有在下标都为整数的情况下才把非数组访问报为 type 8 以避免覆盖 type 7
      bool anyNonInt = false;
      for (size_t idx = 1; idx < children.size(); ++idx) {
        if (getTypeFromAST(children[idx]) != VarType::INT) {
          anyNonInt = true;
          break;
        }
      }
      if (!anyNonInt) {
        reportError(8, line, "subscripted non-array \"" + varName + "\"");
      }
    }
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

  // 参数检查（Error type 9）
  auto funcSym = std::dynamic_pointer_cast<FuncSymbol>(symbol);
  if (!funcSym)
    return;

  // 收集实际参数类型
  std::vector<VarType> actualTypes;
  size_t actualCount = 0;
  for (const auto &c : node->getChildren()) {
    if (c->getName() == "FuncRParams") {
      for (const auto &arg : c->getChildren()) {
        ++actualCount;
        actualTypes.push_back(getTypeFromAST(arg));
      }
    }
  }

  // 参数个数检查
  if (actualCount != funcSym->paramTypes.size()) {
    reportError(9, line,
                "function '" + funcName +
                    "' called with wrong number of arguments");
    return;
  }

  // 参数类型逐一检查
  for (size_t i = 0; i < actualCount; ++i) {
    VarType expected = funcSym->paramTypes[i];
    VarType actual = actualTypes[i];
    if (!typeCompatible(expected, actual)) {
      reportError(9, line,
                  "function '" + funcName + "' argument type mismatch");
      return;
    }
  }
}

void SemanticAnalyzer::visitAssignStmt(const std::unique_ptr<ASTNode> &node) {
  const auto &children = node->getChildren();
  if (children.size() >= 2) {
    // 检查左值（会报告未声明等）
    if (children[0]->getName() == "LVal") {
      visitLVal(children[0]);
    }

    // 检查右值表达式
    if (children.size() > 1) {
      // 触发类型推导，若类型不匹配可在此处扩展检查
      VarType rhs = getTypeFromAST(children[1]);

      // 若左是 ID，可进一步检查赋值类型一致性
      std::string leftName;
      int leftLine = children[0]->getLine();
      if (findFirstID(children[0], leftName, leftLine)) {
        auto sym = lookupSymbol(leftName);
        // 如果左值未声明（sym ==
        // nullptr），不要再做赋值类型检查，避免重复错误（type1 已报告）
        if (sym && !sym->isFunc && rhs != VarType::UNKNOWN) {
          // 计算左值目标类型（如果是数组类型，则目标为元素类型）
          VarType leftTargetType = sym->type;
          if (sym->type == VarType::INT_ARRAY) {
            leftTargetType = VarType::INT;
          }

          // 检查左值是否含有非整数下标（若有，下标错误已报，跳过赋值类型报错以免重复）
          bool leftHasNonIntSubscript = false;
          if (children[0]->getName() == "LVal") {
            const auto &lchildren = children[0]->getChildren();
            if (lchildren.size() > 1) {
              for (size_t k = 1; k < lchildren.size(); ++k) {
                if (getTypeFromAST(lchildren[k]) != VarType::INT) {
                  leftHasNonIntSubscript = true;
                  break;
                }
              }
            }
          }

          if (!leftHasNonIntSubscript && leftTargetType != rhs) {
            reportError(11, node->getLine(), "type mismatched for operands");
          }
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
    visitCompUnit(ast);
  }
}

bool SemanticAnalyzer::hasErrors() { return errorCount > 0; }
