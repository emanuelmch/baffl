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

#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>

llvm::Value *LiteralBooleanAST::generate(EmissionContext &context) const {
  return llvm::ConstantInt::getBool(*context.llvmContext, value);
}

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

llvm::Value *FunctionAST::generate(EmissionContext &context) const {
  auto scopeGuard = context.pushScope();

  const auto boolType = llvm::Type::getInt1Ty(*context.llvmContext);
  const auto i32Type = llvm::Type::getInt32Ty(*context.llvmContext);

  std::vector<llvm::Type *> argumentTypes;
  for (auto argument : arguments) {
    auto argumentType = std::get<1>(argument);
    assert(argumentType == "i32");
    argumentTypes.emplace_back(i32Type);
  }

  // TODO: Move the void main magic elsewhere? Maybe?
  if (this->name == "main") {
    assert(this->returnTypeName == "void" || this->returnTypeName == "i32");
  }

  llvm::Type *returnType;
  if (this->name == "main" || this->returnTypeName == "i32") {
    returnType = i32Type;
  } else if (this->returnTypeName == "bool") {
    returnType = boolType;
  } else {
    assert(this->returnTypeName == "void");
    returnType = llvm::Type::getVoidTy(*context.llvmContext);
  }

  auto functionType = llvm::FunctionType::get(returnType, argumentTypes, false);
  auto functionName = this->name;
  auto function =
      llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, functionName, context.module.get());
  context.addFunction(functionName, function);

  auto entryBlock = llvm::BasicBlock::Create(*context.llvmContext, "entry", function);
  context.builder->SetInsertPoint(entryBlock);

  auto functionArgumentIterator = function->arg_begin();
  auto argumentIterator = this->arguments.cbegin();

  while (functionArgumentIterator != function->arg_end()) {
    assert(argumentIterator != this->arguments.cend());

    auto &functionArgument = (*functionArgumentIterator);
    auto argumentName = std::get<0>(*argumentIterator);
    functionArgument.setName(argumentName);

    // TODO: Could we use a VariableRefAST instead of copying the code?
    auto alloca = context.builder->CreateAlloca(llvm::Type::getInt32Ty(*context.llvmContext), nullptr, argumentName);
    context.builder->CreateStore(&functionArgument, alloca);
    context.addVariable(argumentName, alloca);

    ++functionArgumentIterator;
    ++argumentIterator;
  }

  for (const auto &expression : body) {
    expression->generate(context);
  }

  if (this->returnTypeName == "void") {
    if (this->name == "main") {
      auto integerAst = std::make_shared<LiteralIntegerAST>(0);
      auto returnAst = std::make_shared<ReturnAST>(integerAst);
      returnAst->generate(context);
    } else {
      context.builder->CreateRetVoid();
    }
  }

  context.runPasses(function);

  return function;
}

llvm::Value *FunctionCallAST::generate(EmissionContext &context) const {
  auto function = context.getFunction(this->functionName);
  std::vector<llvm::Value *> argumentValues;
  for (const auto &argument : this->arguments) {
    argumentValues.push_back(argument->generate(context));
  }

  return context.builder->CreateCall(function, argumentValues);
}

llvm::Value *ReturnAST::generate(EmissionContext &context) const {
  auto returnValue = this->value->generate(context);
  return context.builder->CreateRet(returnValue);
}

llvm::Value *PlusOperationAST::generate(EmissionContext &context) const {
  auto leftValue = this->left->generate(context);
  auto rightValue = this->right->generate(context);
  return context.builder->CreateAdd(leftValue, rightValue);
}

llvm::Value *MinusOperationAST::generate(EmissionContext &context) const {
  auto leftValue = this->left->generate(context);
  auto rightValue = this->right->generate(context);
  return context.builder->CreateSub(leftValue, rightValue);
}
