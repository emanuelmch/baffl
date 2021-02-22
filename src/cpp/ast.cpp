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

void LiteralIntegerAST::codegen(llvm::LLVMContext *, const std::shared_ptr<llvm::Module> &, llvm::IRBuilder<> *,
                                llvm::legacy::FunctionPassManager *) const {}

void ReturnAST::codegen(llvm::LLVMContext *, const std::shared_ptr<llvm::Module> &, llvm::IRBuilder<> *,
                        llvm::legacy::FunctionPassManager *) const {}

void FunctionAST::codegen(llvm::LLVMContext *context, const std::shared_ptr<llvm::Module> &module,
                          llvm::IRBuilder<> *builder, llvm::legacy::FunctionPassManager *passManager) const {
  auto functionType = llvm::FunctionType::get(llvm::Type::getInt32Ty(*context), false);
  auto function = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, "main", module.get());

  auto entryBlock = llvm::BasicBlock::Create(*context, "entry", function);
  builder->SetInsertPoint(entryBlock);

  auto retVal = llvm::ConstantInt::get(*context, llvm::APInt(32, 0));
  builder->CreateRet(retVal);
  llvm::verifyFunction(*function);
  passManager->run(*function);
}
