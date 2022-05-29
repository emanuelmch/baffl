/*
 * Copyright (c) 2020 Emanuel Machado da Silva
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include "ast.h"

#include <ostream>

namespace std {

void PrintTo(const ExpressionAST &expressionAST, std::ostream *os);

void PrintTo(const LiteralBooleanAST &literalAST, std::ostream *os) {
  *os << "LiteralBooleanAST {" << literalAST.value << "}";
}

void PrintTo(const LiteralIntegerAST &literalAST, std::ostream *os) {
  *os << "LiteralIntegerAST {" << literalAST.value << "}";
}

void PrintTo(const VariableAssignmentAST &assignmentAst, std::ostream *os) {
  *os << "VarAssignmentAST { " << assignmentAst.varName << " = {";
  PrintTo(*assignmentAst.value, os);
  *os << "}";
}

void PrintTo(const VariableDeclarationAST &varDecAST, std::ostream *os) {
  *os << "VarDecAST { " << (varDecAST.isMutable ? "var" : "let") << " " << varDecAST.varName << " = ";
  PrintTo(*varDecAST.value, os);
  *os << " }";
}

void PrintTo(const VariableReferenceAST &varRefAST, std::ostream *os) {
  *os << "VarRefAST { " << varRefAST.varName << " }";
}

void PrintTo(const FunctionCallAST &functionCallAST, std::ostream *os) {
  *os << "FunctionCallAST { " << functionCallAST.functionName << " }";
}

void PrintTo(const IfAST &ifAST, std::ostream *os) {
  *os << "IfAST { condition = {";
  PrintTo(*ifAST.condition, os);
  *os << "}, then = [" << ifAST.body.size() << "]: {";
  for (const auto &expression : ifAST.body) {
    PrintTo(*expression, os);
    *os << ", ";
  }
  *os << "}";
}

void PrintTo(const ReturnAST &returnAST, std::ostream *os) {
  *os << "ReturnAST { ";
  PrintTo(*returnAST.value, os);
  *os << " }";
}

void PrintTo(const PlusOperationAST &plusAST, std::ostream *os) {
  *os << "BinaryOperation: + { ";
  PrintTo(*plusAST.left, os);
  *os << ", ";
  PrintTo(*plusAST.right, os);
  *os << " }";
}

void PrintTo(const MinusOperationAST &minusAST, std::ostream *os) {
  *os << "BinaryOperation: - { ";
  PrintTo(*minusAST.left, os);
  *os << ", ";
  PrintTo(*minusAST.right, os);
  *os << " }";
}

void PrintTo(const DivisionOperationAST &divisionAST, std::ostream *os) {
  *os << "BinaryOperation: / { ";
  PrintTo(*divisionAST.left, os);
  *os << ", ";
  PrintTo(*divisionAST.right, os);
  *os << " }";
}

void PrintTo(const ModuloOperationAST &moduloAST, std::ostream *os) {
  *os << "BinaryOperation: / { ";
  PrintTo(*moduloAST.left, os);
  *os << ", ";
  PrintTo(*moduloAST.right, os);
  *os << " }";
}

void PrintTo(const EqualsOperationAST &equalsAST, std::ostream *os) {
  *os << "BinaryOperation: == { ";
  PrintTo(*equalsAST.left, os);
  *os << ", ";
  PrintTo(*equalsAST.right, os);
  *os << " }";
}

void PrintTo(const LessThanOperationAST &lessThanAst, std::ostream *os) {
  *os << "BinaryOperation: < { ";
  PrintTo(*lessThanAst.left, os);
  *os << ", ";
  PrintTo(*lessThanAst.right, os);
  *os << " }";
}

void PrintTo(const LessThanOrEqualToOperationAST &lessThanOrEqualToAst, std::ostream *os) {
  *os << "BinaryOperation: <= { ";
  PrintTo(*lessThanOrEqualToAst.left, os);
  *os << ", ";
  PrintTo(*lessThanOrEqualToAst.right, os);
  *os << " }";
}

void PrintTo(const ExpressionAST &expressionAST, std::ostream *os) {
  // FIXME: Drop all of this, let's just have a debug function on the interface
  auto literalBooleanAST = dynamic_cast<const LiteralBooleanAST *>(&expressionAST);
  auto literalIntegerAST = dynamic_cast<const LiteralIntegerAST *>(&expressionAST);
  auto varAssignAST = dynamic_cast<const VariableAssignmentAST *>(&expressionAST);
  auto varDecAST = dynamic_cast<const VariableDeclarationAST *>(&expressionAST);
  auto varRefAST = dynamic_cast<const VariableReferenceAST *>(&expressionAST);
  auto functionCallAST = dynamic_cast<const FunctionCallAST *>(&expressionAST);
  auto ifAST = dynamic_cast<const IfAST *>(&expressionAST);
  auto returnAST = dynamic_cast<const ReturnAST *>(&expressionAST);
  auto plusOperationAST = dynamic_cast<const PlusOperationAST *>(&expressionAST);
  auto minusOperationAST = dynamic_cast<const MinusOperationAST *>(&expressionAST);
  auto divisionOperationAST = dynamic_cast<const DivisionOperationAST*>(&expressionAST);
  auto moduloOperationAST = dynamic_cast<const ModuloOperationAST*>(&expressionAST);
  auto equalsOperationAST = dynamic_cast<const EqualsOperationAST *>(&expressionAST);
  auto lessThanOperationAST = dynamic_cast<const LessThanOperationAST *>(&expressionAST);
  auto lessThanOrEqualToOperationAST = dynamic_cast<const LessThanOrEqualToOperationAST *>(&expressionAST);

  if (literalBooleanAST) {
    PrintTo(*literalBooleanAST, os);
  } else if (literalIntegerAST) {
    PrintTo(*literalIntegerAST, os);
  } else if (varAssignAST) {
    PrintTo(*varAssignAST, os);
  } else if (varDecAST) {
    PrintTo(*varDecAST, os);
  } else if (varRefAST) {
    PrintTo(*varRefAST, os);
  } else if (functionCallAST) {
    PrintTo(*functionCallAST, os);
  } else if (ifAST) {
    PrintTo(*ifAST, os);
  } else if (returnAST) {
    PrintTo(*returnAST, os);
  } else if (plusOperationAST) {
    PrintTo(*plusOperationAST, os);
  } else if (minusOperationAST) {
    PrintTo(*minusOperationAST, os);
  } else if (divisionOperationAST) {
    PrintTo(*divisionOperationAST, os);
  } else if (moduloOperationAST) {
    PrintTo(*moduloOperationAST, os);
  } else if (equalsOperationAST) {
    PrintTo(*equalsOperationAST, os);
  } else if (lessThanOperationAST) {
    PrintTo(*lessThanOperationAST, os);
  } else if (lessThanOrEqualToOperationAST) {
    PrintTo(*lessThanOrEqualToOperationAST, os);
  } else {
    *os << "Unknown Expression AST!!!";
  }
}

void PrintTo(const FunctionAST &functionAST, std::ostream *os) {
  *os << "FunctionAST { " << functionAST.name << ": " << functionAST.returnTypeName << ", arguments { ";
  for (const auto &argument : functionAST.realArguments) {
    const auto &[name, type, value, isMutable] = argument;
    *os << name << ": " << (isMutable ? "Mut " : "") << type << ", ";
  }
  *os << "}, expressions[" << functionAST.body.size() << "]: { ";
  for (const auto &expression : functionAST.body) {
    PrintTo(*expression, os);
    *os << ", ";
  }
  *os << " } }";
}

void PrintTo(const TopLevelAST &topLevelAST, std::ostream *os) {
  auto functionAST = dynamic_cast<const FunctionAST *>(&topLevelAST);

  if (functionAST) {
    PrintTo(*functionAST, os);
  } else {
    *os << "Unknown Top Level AST!!!";
  }
}

void PrintTo(const AST &ast, std::ostream *os) {
  auto topLevelAST = dynamic_cast<const TopLevelAST *>(&ast);
  if (topLevelAST) {
    PrintTo(*topLevelAST, os);
    return;
  }

  auto expressionAST = dynamic_cast<const ExpressionAST *>(&ast);
  if (expressionAST) {
    PrintTo(*expressionAST, os);
    return;
  }

  *os << "Unknown AST!!!";
}

void PrintTo(const std::shared_ptr<TopLevelAST> &ast, std::ostream *os) {
  PrintTo(*ast, os);
}
} // namespace std

// TODO: I'm pretty sure these aren't needed anymore
bool operator==(const std::shared_ptr<TopLevelAST> &l, const std::shared_ptr<TopLevelAST> &r) {
  return ((!l && !r) || (l && r && *l == *r));
}

bool operator==(const std::shared_ptr<TopLevelAST> &l, const std::shared_ptr<AST> &r) {
  return ((!l && !r) || (l && r && *l == *r));
}
