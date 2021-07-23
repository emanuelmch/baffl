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

void PrintTo(const LiteralIntegerAST &literalAST, std::ostream *os) {
  *os << "LiteralIntegerAST {" << literalAST.value << "}";
}

void PrintTo(const VariableDeclarationAST &varDecAST, std::ostream *os) {
  *os << "VarDecAST { " << varDecAST.varName << ", ";
  PrintTo(*varDecAST.value, os);
  *os << " }";
}

void PrintTo(const VariableReferenceAST &varRefAST, std::ostream *os) {
  *os << "VarRefAST { " << varRefAST.varName << " }";
}

void PrintTo(const FunctionCallAST &functionCallAST, std::ostream *os) {
  *os << "FunctionCallAST { " << functionCallAST.functionName << " }";
}

void PrintTo(const ReturnAST &returnAST, std::ostream *os) {
  *os << "ReturnAST { ";
  PrintTo(*returnAST.value, os);
  *os << " }";
}

void PrintTo(const PlusOperationAST &plusAST, std::ostream *os) {
  *os << "PlusOperationAST { ";
  PrintTo(*plusAST.left, os);
  *os << ", ";
  PrintTo(*plusAST.right, os);
  *os << " }";
}

void PrintTo(const MinusOperationAST &minusAST, std::ostream *os) {
  *os << "MinusOperationAST { ";
  PrintTo(*minusAST.left, os);
  *os << ", ";
  PrintTo(*minusAST.right, os);
  *os << " }";
}

void PrintTo(const ExpressionAST &expressionAST, std::ostream *os) {
  auto literalIntegerAST = dynamic_cast<const LiteralIntegerAST *>(&expressionAST);
  auto varDecAST = dynamic_cast<const VariableDeclarationAST *>(&expressionAST);
  auto varRefAST = dynamic_cast<const VariableReferenceAST *>(&expressionAST);
  auto functionCallAST = dynamic_cast<const FunctionCallAST *>(&expressionAST);
  auto returnAST = dynamic_cast<const ReturnAST *>(&expressionAST);
  auto plusOperationAST = dynamic_cast<const PlusOperationAST *>(&expressionAST);
  auto minusOperationAST = dynamic_cast<const MinusOperationAST *>(&expressionAST);

  if (literalIntegerAST) {
    PrintTo(*literalIntegerAST, os);
  } else if (varDecAST) {
    PrintTo(*varDecAST, os);
  } else if (varRefAST) {
    PrintTo(*varRefAST, os);
  } else if (functionCallAST) {
    PrintTo(*functionCallAST, os);
  } else if (returnAST) {
    PrintTo(*returnAST, os);
  } else if (plusOperationAST) {
    PrintTo(*plusOperationAST, os);
  } else if (minusOperationAST) {
    PrintTo(*minusOperationAST, os);
  } else {
    *os << "Unknown Expression AST!!!";
  }
}

void PrintTo(const FunctionAST &functionAST, std::ostream *os) {
  *os << "FunctionAST { " << functionAST.name << ": " << functionAST.returnType << ", arguments { ";
  for (const auto &argument : functionAST.arguments) {
    const auto &[name, type] = argument;
    *os << name << ": " << type << ", ";
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
