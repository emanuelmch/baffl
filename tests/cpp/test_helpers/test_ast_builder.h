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

struct ExpressionBuilder {

  virtual ~ExpressionBuilder() = default;

  // By implementing binary operators like this, we ensure we get the left-first precedence on multiple operations
  std::shared_ptr<ExpressionBuilder> plus(const std::shared_ptr<ExpressionBuilder> &);
  std::shared_ptr<ExpressionBuilder> minus(const std::shared_ptr<ExpressionBuilder> &);
  std::shared_ptr<ExpressionBuilder> modulo(const std::shared_ptr<ExpressionBuilder> &);
  std::shared_ptr<ExpressionBuilder> equals(const std::shared_ptr<ExpressionBuilder> &);
  std::shared_ptr<ExpressionBuilder> lessThan(const std::shared_ptr<ExpressionBuilder> &);
  std::shared_ptr<ExpressionBuilder> lessThanOrEqualTo(const std::shared_ptr<ExpressionBuilder> &);

  virtual std::shared_ptr<ExpressionBuilder> to_shared() = 0;
  virtual std::shared_ptr<ExpressionAST> build() = 0;
};

struct BinaryOperatorExpressionBuilder : ExpressionBuilder {
  const TokenType _operator;
  const std::shared_ptr<ExpressionBuilder> first;
  const std::shared_ptr<ExpressionBuilder> second;

  BinaryOperatorExpressionBuilder(const TokenType _operator, std::shared_ptr<ExpressionBuilder> first,
                                  std::shared_ptr<ExpressionBuilder> second)
      : _operator(_operator), first(std::move(first)), second(std::move(second)) {}
  ~BinaryOperatorExpressionBuilder() override = default;

  std::shared_ptr<ExpressionBuilder> to_shared() override {
    return std::make_shared<BinaryOperatorExpressionBuilder>(*this);
  }

  std::shared_ptr<ExpressionAST> build() override {
    auto left = first->build();
    auto right = second->build();
    switch (_operator) {
    case operator_plus:
      return std::make_shared<PlusOperationAST>(left, right);
    case operator_minus:
      return std::make_shared<MinusOperationAST>(left, right);
    case operator_modulo:
      return std::make_shared<ModuloOperationAST>(left, right);
    case operator_equals:
      return std::make_shared<EqualsOperationAST>(left, right);
    case operator_less_than:
      return std::make_shared<LessThanOperationAST>(left, right);
    case operator_less_than_or_equal_to:
      return std::make_shared<LessThanOrEqualToOperationAST>(left, right);
    default:
      assert(!"Unknown operator");
      return std::make_shared<PlusOperationAST>(left, right);
    }
  }
};

inline std::shared_ptr<ExpressionBuilder> ExpressionBuilder::plus(const std::shared_ptr<ExpressionBuilder> &right) {
  const std::shared_ptr<ExpressionBuilder> first = to_shared();
  const std::shared_ptr<ExpressionBuilder> &second = right;
  return std::make_shared<BinaryOperatorExpressionBuilder>(operator_plus, first, second);
}

inline std::shared_ptr<ExpressionBuilder> ExpressionBuilder::minus(const std::shared_ptr<ExpressionBuilder> &right) {
  const std::shared_ptr<ExpressionBuilder> first = to_shared();
  const std::shared_ptr<ExpressionBuilder> &second = right;
  return std::make_shared<BinaryOperatorExpressionBuilder>(operator_minus, first, second);
}

inline std::shared_ptr<ExpressionBuilder> ExpressionBuilder::modulo(const std::shared_ptr<ExpressionBuilder> &right) {
  const std::shared_ptr<ExpressionBuilder> first = to_shared();
  const std::shared_ptr<ExpressionBuilder> &second = right;
  return std::make_shared<BinaryOperatorExpressionBuilder>(operator_modulo, first, second);
}
inline std::shared_ptr<ExpressionBuilder> ExpressionBuilder::equals(const std::shared_ptr<ExpressionBuilder> &right) {
  auto first = to_shared();
  auto &second = right;
  return std::make_shared<BinaryOperatorExpressionBuilder>(operator_equals, first, second);
}

inline std::shared_ptr<ExpressionBuilder> ExpressionBuilder::lessThan(const std::shared_ptr<ExpressionBuilder> &right) {
  auto first = to_shared();
  auto &second = right;
  return std::make_shared<BinaryOperatorExpressionBuilder>(operator_less_than, first, second);
}

inline std::shared_ptr<ExpressionBuilder> ExpressionBuilder::lessThanOrEqualTo(const std::shared_ptr<ExpressionBuilder> &right) {
  auto first = to_shared();
  auto &second = right;
  return std::make_shared<BinaryOperatorExpressionBuilder>(operator_less_than_or_equal_to, first, second);
}

struct BoolLiteralExpressionBuilder : ExpressionBuilder {
  const bool value;

  explicit BoolLiteralExpressionBuilder(bool value) : value(value) {}
  ~BoolLiteralExpressionBuilder() override = default;

  inline std::shared_ptr<ExpressionBuilder> to_shared() override {
    return std::make_shared<BoolLiteralExpressionBuilder>(*this);
  }
  inline std::shared_ptr<ExpressionAST> build() override { return std::make_shared<LiteralBooleanAST>(value); }
};

struct IntLiteralExpressionBuilder : ExpressionBuilder {
  const uint64_t value;

  explicit IntLiteralExpressionBuilder(uint64_t value) : value(value) {}
  ~IntLiteralExpressionBuilder() override = default;

  inline std::shared_ptr<ExpressionBuilder> to_shared() override {
    return std::make_shared<IntLiteralExpressionBuilder>(*this);
  }
  inline std::shared_ptr<ExpressionAST> build() override { return std::make_shared<LiteralIntegerAST>(value); }
};

struct VariableExpressionBuilder : ExpressionBuilder {
  const std::string name;

  explicit VariableExpressionBuilder(std::string name) : name(std::move(name)) {}
  ~VariableExpressionBuilder() override = default;

  inline std::shared_ptr<ExpressionBuilder> to_shared() override {
    return std::make_shared<VariableExpressionBuilder>(*this);
  }
  inline std::shared_ptr<ExpressionAST> build() override { return std::make_shared<VariableReferenceAST>(name); }
};

inline std::shared_ptr<ExpressionBuilder> boolLiteral(bool value) {
  return std::make_shared<BoolLiteralExpressionBuilder>(value);
}

inline std::shared_ptr<ExpressionBuilder> intLiteral(uint64_t value) {
  return std::make_shared<IntLiteralExpressionBuilder>(value);
}

inline std::shared_ptr<ExpressionBuilder> variable(const std::string &name) {
  return std::make_shared<VariableExpressionBuilder>(name);
}

struct ASTBuilder {
  static inline ASTBuilder block() { return ASTBuilder{}; }

  static inline ASTBuilder import(const std::string &name) {
    ASTBuilder builder;
    builder.isImport = true;
    builder.functionName = name;
    return builder;
  }

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

  inline ASTBuilder ifExpression(const std::function<std::shared_ptr<ExpressionBuilder>()> &condition,
                                 const std::function<void(ASTBuilder *)> &positiveBranch) {
    auto conditionAst = condition()->build();

    auto innerBody = ASTBuilder::block();
    positiveBranch(&innerBody);

    auto ifAst = std::make_shared<IfAST>(conditionAst, innerBody.body);
    body.push_back(ifAst);

    return *this;
  }

  inline ASTBuilder whileExpression(const std::function<std::shared_ptr<ExpressionBuilder>()> &condition,
                                 const std::function<void(ASTBuilder *)> &loopBody) {
    auto conditionAst = condition()->build();

    // TODO: Maybe we
    auto innerBody = ASTBuilder::block();
    loopBody(&innerBody);

    auto whileAst = std::make_shared<WhileAST>(conditionAst, innerBody.body);
    body.push_back(whileAst);

    return *this;
  }

  inline ASTBuilder declareVariable(const std::string &varName, uint64_t value) {
    auto literalAst = std::make_shared<LiteralIntegerAST>(value);
    auto declarationAst = std::make_shared<VariableDeclarationAST>(varName, literalAst);
    body.push_back(declarationAst);

    return *this;
  }

  inline ASTBuilder declareVariable(const std::string &varName, const std::string &value) {
    auto literalAst = std::make_shared<LiteralStringAST>(value);
    auto declarationAst = std::make_shared<VariableDeclarationAST>(varName, literalAst);
    body.push_back(declarationAst);

    return *this;
  }

  inline ASTBuilder declareMutableVariable(const std::string &varName, uint64_t value) {
    auto literalAst = std::make_shared<LiteralIntegerAST>(value);
    auto declarationAst = std::make_shared<VariableDeclarationAST>(varName, literalAst, true);
    body.push_back(declarationAst);

    return *this;
  }

  inline ASTBuilder assignVariable(const std::string &varName, uint64_t value) {
    auto literalAst = std::make_shared<LiteralIntegerAST>(value);
    auto assignmentAst = std::make_shared<VariableAssignmentAST>(varName, literalAst);
    body.push_back(assignmentAst);

    return *this;
  }

  inline ASTBuilder returnBoolLiteral(bool value) {
    auto literalAst = std::make_shared<LiteralBooleanAST>(value);
    auto returnAst = std::make_shared<ReturnAST>(literalAst);
    body.push_back(returnAst);

    return *this;
  }

  inline ASTBuilder returnIntLiteral(uint64_t value) {
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

  inline ASTBuilder returnFunctionCall(std::string name, uint64_t arg0) {
    auto integerLiteralAst = std::make_shared<LiteralIntegerAST>(arg0);
    std::vector<std::shared_ptr<ExpressionAST>> arguments{integerLiteralAst};
    auto functionCallAst = std::make_shared<FunctionCallAST>(std::move(name), arguments);
    auto returnAst = std::make_shared<ReturnAST>(functionCallAst);
    body.push_back(returnAst);

    return *this;
  }

  // TODO: Use var args instead of multiple overloads
  inline ASTBuilder returnFunctionCall(std::string name, uint64_t arg0, uint64_t arg1, uint64_t arg2) {
    auto arg0Ast = std::make_shared<LiteralIntegerAST>(arg0);
    auto arg1Ast = std::make_shared<LiteralIntegerAST>(arg1);
    auto arg2Ast = std::make_shared<LiteralIntegerAST>(arg2);
    std::vector<std::shared_ptr<ExpressionAST>> arguments{arg0Ast, arg1Ast, arg2Ast};
    auto functionCallAst = std::make_shared<FunctionCallAST>(std::move(name), arguments);
    auto returnAst = std::make_shared<ReturnAST>(functionCallAst);
    body.push_back(returnAst);

    return *this;
  }

  inline ASTBuilder returnExpression(const std::function<std::shared_ptr<ExpressionBuilder>()> &lambda) {
    auto expressionAst = lambda()->build();
    auto returnAst = std::make_shared<ReturnAST>(expressionAst);
    body.push_back(returnAst);

    return *this;
  }

  [[nodiscard]] inline std::shared_ptr<AST> build() const {
    if (isImport) {
      return std::make_shared<ImportAST>(functionName);
    }
    return std::make_shared<FunctionAST>(functionName, functionReturnType, functionArguments, body);
  }

private:
  bool isImport = false;
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
}
