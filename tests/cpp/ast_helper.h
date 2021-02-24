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

#include "cpp/ast.h"

#include <ostream>

namespace std {

void PrintTo(const ExpressionAST &expressionAST, std::ostream *os);

void PrintTo(const LiteralIntegerAST &literalAST, std::ostream *os) {
  *os << "LiteralIntegerAST {" << literalAST.value << "}";
}

void PrintTo(const ReturnAST &returnAST, std::ostream *os) {
  *os << "ReturnAST { ";
  PrintTo(*returnAST.value, os);
  *os << " }";
}

void PrintTo(const ExpressionAST &expressionAST, std::ostream *os) {
  auto literalIntegerAST = dynamic_cast<const LiteralIntegerAST *>(&expressionAST);
  auto returnAST = dynamic_cast<const ReturnAST *>(&expressionAST);

  if (literalIntegerAST) {
    PrintTo(*literalIntegerAST, os);
  } else if (returnAST) {
    PrintTo(*returnAST, os);
  } else {
    *os << "Unknown Expression AST!!!";
  }
}

void PrintTo(const FunctionAST &functionAST, std::ostream *os) {
  *os << "FunctionAST { name: [" << functionAST.name << "], expression: { ";
  if (functionAST.body == nullptr) {
    *os << "nullptr";
  } else {
    PrintTo(*functionAST.body, os);
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
}

bool operator==(const std::shared_ptr<TopLevelAST> &l, const std::shared_ptr<TopLevelAST> &r) {
  return ((!l && !r) || (l && r && *l == *r));
}
