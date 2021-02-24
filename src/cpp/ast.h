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

#include "code_lexer.h"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>

#include <cstdint>
#include <memory>
#include <utility>

// Interfaces
struct EmissionContext {
  std::shared_ptr<llvm::LLVMContext> llvmContext;
  std::shared_ptr<llvm::Module> module;
  std::shared_ptr<llvm::IRBuilder<>> builder;
  std::shared_ptr<llvm::legacy::FunctionPassManager> passManager;
};

struct AST {
  virtual llvm::Value *generate(const EmissionContext &) const = 0;

  virtual bool operator==(const AST &) const = 0;
};

struct TopLevelAST : public AST {
  virtual ~TopLevelAST() = default;
};

struct ExpressionAST : public AST {};

// Concrete
struct LiteralIntegerAST : public ExpressionAST {
  const uint64_t value;

  explicit LiteralIntegerAST(uint64_t value) : value(value) {}
  virtual ~LiteralIntegerAST() = default;

  llvm::Value *generate(const EmissionContext &) const override;

  bool operator==(const AST &o) const override {
    auto other = dynamic_cast<const LiteralIntegerAST *>(&o);
    return other && this->value == other->value;
  }
};

struct ReturnAST : public ExpressionAST {
  const std::shared_ptr<const ExpressionAST> value;

  explicit ReturnAST(std::shared_ptr<const ExpressionAST> value) : value(std::move(value)) {}
  virtual ~ReturnAST() = default;

  llvm::Value *generate(const EmissionContext &) const override;

  bool operator==(const AST &o) const override {
    auto other = dynamic_cast<const ReturnAST *>(&o);
    return other && *(this->value) == *(other->value);
  }
};

struct FunctionAST : TopLevelAST {
  const std::string name;
  const std::string returnType;
  const std::shared_ptr<const ExpressionAST> body;

  FunctionAST(std::string name, std::string returnType, std::shared_ptr<const ExpressionAST> body)
      : name(std::move(name)), returnType(std::move(returnType)), body(std::move(body)) {}
  ~FunctionAST() override = default;

  llvm::Value *generate(const EmissionContext &) const override;

  bool operator==(const AST &o) const override {
    auto other = dynamic_cast<const FunctionAST *>(&o);
    return other && this->name == other->name && *(this->body) == *(other->body);
  }
};
