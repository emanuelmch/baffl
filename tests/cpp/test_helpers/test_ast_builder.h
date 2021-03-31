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

#include <utility>

#include "ast.h"

#include "ast.h"

struct ExpressionBuilder {

  inline ExpressionBuilder literal(uint64_t value) {
    this->first = value;
    return *this;
  }

  inline ExpressionBuilder plus(ExpressionBuilder left, ExpressionBuilder right) {
    this->first = left.first;
    this->second = right.first;
    return *this;
  }

  inline std::shared_ptr<ExpressionAST> build() {
    auto left = std::make_shared<LiteralIntegerAST>(first);
    auto right = std::make_shared<LiteralIntegerAST>(second);
    return std::make_shared<PlusOperationAST>(left, right);
  }

private:
  uint64_t first;
  uint64_t second;
};

struct ASTBuilder {
  static inline ASTBuilder function(const std::string &name, const std::string &returnType) {
    ASTBuilder builder;
    builder.functionName = name;
    builder.functionReturnType = returnType;
    return builder;
  }

  inline ASTBuilder addArgument(const std::string &name, const std::string &type) {
    functionArguments.emplace_back(name, type);

    return *this;
  }

  inline ASTBuilder declareVariable(const std::string &varName, uint64_t value) {
    auto literalAst = std::make_shared<LiteralIntegerAST>(value);
    VariableDeclarationAST test(varName, literalAst);
    auto declarationAst = std::make_shared<VariableDeclarationAST>(varName, literalAst);
    body.push_back(declarationAst);

    return *this;
  }

  inline ASTBuilder returnLiteral(uint64_t value) {
    auto literalAst = std::make_shared<LiteralIntegerAST>(value);
    auto returnAst = std::make_shared<ReturnAST>(literalAst);
    body.push_back(returnAst);

    return *this;
  }

  inline ASTBuilder returnVariable(std::string varName) {
    auto varReferenceAst = std::make_shared<VariableReferenceAST>(std::move(varName));
    auto returnAst = std::make_shared<ReturnAST>(varReferenceAst);
    body.push_back(returnAst);

    return *this;
  }

  inline ASTBuilder returnFunctionCall(std::string name) {
    auto functionCallAst = std::make_shared<FunctionCallAST>(std::move(name));
    auto returnAst = std::make_shared<ReturnAST>(functionCallAst);
    body.push_back(returnAst);

    return *this;
  }

  inline ASTBuilder returnFunctionCall(std::string name, uint64_t argument0) {
    auto integerLiteralAst = std::make_shared<LiteralIntegerAST>(argument0);
    std::vector<std::shared_ptr<ExpressionAST>> arguments{integerLiteralAst};
    auto functionCallAst = std::make_shared<FunctionCallAST>(std::move(name), arguments);
    auto returnAst = std::make_shared<ReturnAST>(functionCallAst);
    body.push_back(returnAst);

    return *this;
  }

  inline ASTBuilder returnExpression(const std::function<ExpressionBuilder(ExpressionBuilder)> &lambda) {
    auto expressionAst = lambda({}).build();
    auto returnAst = std::make_shared<ReturnAST>(expressionAst);
    body.push_back(returnAst);

    return *this;
  }

  [[nodiscard]] inline std::shared_ptr<AST> build() const {
    return std::make_shared<FunctionAST>(functionName, functionReturnType, functionArguments, body);
  }

private:
  std::string functionName;
  std::string functionReturnType;
  std::vector<std::tuple<std::string, std::string>> functionArguments;
  std::vector<std::shared_ptr<const ExpressionAST>> body;
};

bool operator==(const std::shared_ptr<TopLevelAST> &ast, const ASTBuilder &builder) {
  if (!ast) return false;
  return *ast == *(builder.build());
}

namespace std {

void PrintTo(const ASTBuilder &astBuilder, std::ostream *os) {
  PrintTo(*astBuilder.build(), os);
}
};
