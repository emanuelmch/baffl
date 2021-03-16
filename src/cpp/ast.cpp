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

#include "ast.h"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Constants.h>

llvm::Value *LiteralIntegerAST::generate(EmissionContext &context) const {
  return llvm::ConstantInt::get(*context.llvmContext, llvm::APInt(32, this->value));
}

llvm::Value *VariableDeclarationAST::generate(EmissionContext &context) const {
  auto function = context.builder->GetInsertBlock()->getParent();

  llvm::IRBuilder<> entryBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());
  auto alloca = entryBuilder.CreateAlloca(llvm::Type::getInt32Ty(*context.llvmContext), nullptr, this->varName);
  context.addVariable(this->varName, alloca);

  assert(this->value);
  auto initialValue = this->value->generate(context);
  context.builder->CreateStore(initialValue, alloca);

  return alloca;
}

llvm::Value *VariableReferenceAST::generate(EmissionContext &context) const {
  auto variable = context.getVariable(this->varName);
  return context.builder->CreateLoad(variable);
}

llvm::Value *ReturnAST::generate(EmissionContext &context) const {
  auto returnValue = this->value->generate(context);
  return context.builder->CreateRet(returnValue);
}

llvm::Value *FunctionAST::generate(EmissionContext &context) const {
  auto functionType = llvm::FunctionType::get(llvm::Type::getInt32Ty(*context.llvmContext), false);
  auto function = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, "main", context.module.get());

  auto entryBlock = llvm::BasicBlock::Create(*context.llvmContext, "entry", function);
  context.builder->SetInsertPoint(entryBlock);

  for (const auto& expression : body) {
    expression->generate(context);
  }

  context.runPasses(function);
  return function;
}
