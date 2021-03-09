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

#include <llvm/IR/Verifier.h>

EmissionContext::EmissionContext(std::shared_ptr<llvm::LLVMContext> context)
    : llvmContext(std::move(context)), builder(std::make_shared<llvm::IRBuilder<>>(*llvmContext)),
      module(std::make_shared<llvm::Module>("baffl_main", *llvmContext)), passManager(module.get()) {
  passManager.doInitialization();
}

bool EmissionContext::runPasses(llvm::Function *function) {
  auto verifyFailed = llvm::verifyFunction(*function, &llvm::errs());
  auto runFailed = passManager.run(*function);
  return verifyFailed || runFailed;
}

llvm::Value *LiteralIntegerAST::generate(EmissionContext &context) const {
  return llvm::ConstantInt::get(*context.llvmContext, llvm::APInt(32, this->value));
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

  for (auto expression : body) {
    expression->generate(context);
  }

  context.runPasses(function);
  return function;
}
