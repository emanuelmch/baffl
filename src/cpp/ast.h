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

#include "ast/emission_context.h"
#include "code_lexer.h"

#include <llvm/IR/Value.h>

#include <utility>

// Interfaces
struct AST {
  virtual llvm::Value *generate(EmissionContext &) const = 0;

  virtual bool operator==(const AST &) const = 0;
};

struct TopLevelAST : public AST {
  virtual ~TopLevelAST() = default;
};

struct ExpressionAST : public AST {
  virtual ~ExpressionAST() = default;
};

// Literals
struct LiteralIntegerAST : public ExpressionAST {
  const uint64_t value;

  explicit LiteralIntegerAST(uint64_t value) : value(value) {}
  ~LiteralIntegerAST() override = default;

  llvm::Value *generate(EmissionContext &) const override;

  inline bool operator==(const AST &o) const override {
    auto other = dynamic_cast<const LiteralIntegerAST *>(&o);
    return other && this->value == other->value;
  }
};

// Variables
struct VariableDeclarationAST : public ExpressionAST {
  const std::string varName;
  const std::shared_ptr<ExpressionAST> value;

  VariableDeclarationAST(std::string varName, std::shared_ptr<ExpressionAST> value)
      : varName(std::move(varName)), value(std::move(value)) {}

  llvm::Value *generate(EmissionContext &) const override;

  inline bool operator==(const AST &o) const override {
    auto other = dynamic_cast<const VariableDeclarationAST *>(&o);
    return other && this->varName == other->varName && *(this->value) == *(other->value);
  }
};

struct VariableReferenceAST : public ExpressionAST {
  const std::string varName;

  explicit VariableReferenceAST(std::string varName) : varName(std::move(varName)) {}

  llvm::Value *generate(EmissionContext &) const override;

  inline bool operator==(const AST &o) const override {
    auto other = dynamic_cast<const VariableReferenceAST *>(&o);
    return other && this->varName == other->varName;
  }
};

// Functions
struct FunctionAST : public TopLevelAST {
  const std::string name;
  const std::string returnType;
  const std::vector<std::tuple<std::string, std::string>> arguments;
  const std::vector<std::shared_ptr<const ExpressionAST>> body;

  FunctionAST(std::string name, std::string returnType, std::vector<std::tuple<std::string, std::string>> arguments,
              std::vector<std::shared_ptr<const ExpressionAST>> body)
      : name(std::move(name)), returnType(std::move(returnType)), arguments(std::move(arguments)),
        body(std::move(body)) {}
  ~FunctionAST() override = default;

  llvm::Value *generate(EmissionContext &) const override;

  inline bool operator==(const AST &o) const override {
    auto other = dynamic_cast<const FunctionAST *>(&o);
    return other && this->name == other->name && this->returnType == other->returnType &&
           compareBodies(this->body, other->body);
  }

  inline static bool compareBodies(const std::vector<std::shared_ptr<const ExpressionAST>> &left,
                                   const std::vector<std::shared_ptr<const ExpressionAST>> &right) {
    if (left.size() != right.size()) return false;

    for (auto l = left.cbegin(), r = right.cbegin(); l != left.cend(); ++l, ++r) {
      auto leftItem = l->get();
      auto rightItem = r->get();
      // TODO: Replace this with `*leftItem != *rightItem` on C++20
      if (!leftItem->operator==(*rightItem)) {
        return false;
      }
    }

    return true;
  }
};

struct FunctionCallAST : public ExpressionAST {
  const std::string functionName;
  const std::vector<std::shared_ptr<ExpressionAST>> arguments;

  explicit FunctionCallAST(std::string functionName, std::vector<std::shared_ptr<ExpressionAST>> arguments = {})
      : functionName(std::move(functionName)), arguments(std::move(arguments)) {}
  ~FunctionCallAST() override = default;

  llvm::Value *generate(EmissionContext &context) const override;

  bool operator==(const AST &o) const override {
    auto other = dynamic_cast<const FunctionCallAST *>(&o);
    return other && this->functionName == other->functionName;
  }
};

// Operations
struct ReturnAST : public ExpressionAST {
  const std::shared_ptr<const ExpressionAST> value;

  explicit ReturnAST(std::shared_ptr<const ExpressionAST> value) : value(std::move(value)) {}
  ~ReturnAST() override = default;

  llvm::Value *generate(EmissionContext &) const override;

  inline bool operator==(const AST &o) const override {
    auto other = dynamic_cast<const ReturnAST *>(&o);
    return other && *(this->value) == *(other->value);
  }
};

struct PlusOperationAST : public ExpressionAST {
  const std::shared_ptr<const ExpressionAST> left;
  const std::shared_ptr<const ExpressionAST> right;

  PlusOperationAST(std::shared_ptr<const ExpressionAST> left, std::shared_ptr<const ExpressionAST> right)
      : left(std::move(left)), right(std::move(right)) {}

  llvm::Value *generate(EmissionContext &) const override;

  inline bool operator==(const AST &o) const override {
    auto other = dynamic_cast<const PlusOperationAST *>(&o);
    return other && *(this->left) == *(other->left) && *(this->right) == *(other->right);
  }
};
