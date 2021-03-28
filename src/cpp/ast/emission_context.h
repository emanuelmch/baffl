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

#include "helpers/raii.h"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>

#include <cassert>
#include <map>
#include <memory>
#include <utility>

// FIXME: Remove this line, but CLion is driving me crazy
typedef u_int64_t uint64_t;

struct Scope {
  Scope *parent;

  explicit Scope(Scope *parent) : parent(parent) {}

  inline void addVariable(const std::string &name, llvm::AllocaInst *alloca) {
    assert(variables.count(name) == 0);
    variables[name] = alloca;
  }

  inline llvm::AllocaInst *getVariable(const std::string &name) {
    if (variables.count(name)) {
      return variables[name];
    }

    assert(parent != nullptr);
    return parent->getVariable(name);
  }

private:
  std::map<std::string, llvm::AllocaInst *> variables;
};

struct EmissionContext {
  std::shared_ptr<llvm::LLVMContext> llvmContext;
  std::shared_ptr<llvm::IRBuilder<>> builder;
  std::shared_ptr<llvm::Module> module;

  explicit EmissionContext(std::shared_ptr<llvm::LLVMContext>);
  bool runPasses(llvm::Function *);

  inline RunnerScopeGuard pushScope() {
    auto level = ++currentLevel;
    this->scope = new Scope{this->scope};
    return RunnerScopeGuard{[this, level] {
      assert(currentLevel == level);
      scope = scope->parent;
      currentLevel--;
    }};
  }

  inline void addVariable(const std::string &name, llvm::AllocaInst *alloca) {
    assert(scope != nullptr);
    scope->addVariable(name, alloca);
  }

  inline llvm::AllocaInst *getVariable(const std::string &name) {
    assert(scope != nullptr);
    return scope->getVariable(name);
  }

  inline void addFunction(const std::string &name, llvm::Function *function) {
    assert(functions.count(name) == 0);
    functions[name] = function;
  }

  inline llvm::Function *getFunction(const std::string &name) {
    assert(functions.count(name));
    return functions[name];
  }

private:
  Scope *scope = nullptr;
  uint_fast8_t currentLevel = 0;
  std::map<std::string, llvm::Function *> functions;
  llvm::legacy::FunctionPassManager passManager;
};
