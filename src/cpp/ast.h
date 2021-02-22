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
struct AST {
  virtual void generate(llvm::LLVMContext *, const std::shared_ptr<llvm::Module> &, llvm::IRBuilder<> *,
                       llvm::legacy::FunctionPassManager *) const = 0;
};

struct TopLevelAST : public AST {
  virtual ~TopLevelAST() = default;

  virtual bool operator==(const TopLevelAST &) const = 0;
};

struct ExpressionAST : public AST {};

// Concrete
struct LiteralIntegerAST : public ExpressionAST {
  const intmax_t value;

  explicit LiteralIntegerAST(intmax_t value) : value(value) {}
  virtual ~LiteralIntegerAST() = default;

  void generate(llvm::LLVMContext *, const std::shared_ptr<llvm::Module> &, llvm::IRBuilder<> *,
               llvm::legacy::FunctionPassManager *) const override;
};

struct ReturnAST : public ExpressionAST {
  const std::shared_ptr<const ExpressionAST> value;

  explicit ReturnAST(std::shared_ptr<const ExpressionAST> value) : value(std::move(value)) {}
  virtual ~ReturnAST() = default;

  void generate(llvm::LLVMContext *, const std::shared_ptr<llvm::Module> &, llvm::IRBuilder<> *,
               llvm::legacy::FunctionPassManager *) const override;
};

struct FunctionAST : TopLevelAST {
  const std::string name;
  const std::shared_ptr<const ExpressionAST> body;

  FunctionAST(std::string name, std::shared_ptr<const ExpressionAST> body) : name(std::move(name)), body(std::move(body)) {}
  ~FunctionAST() override = default;

  void generate(llvm::LLVMContext *, const std::shared_ptr<llvm::Module> &, llvm::IRBuilder<> *,
               llvm::legacy::FunctionPassManager *) const override;

  // TODO: Fix this
  bool operator==(const TopLevelAST &) const override { return true;}
};
