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
#include <llvm/IR/InlineAsm.h>

llvm::Value *LiteralBooleanAST::generate(EmissionContext &context) const {
  return llvm::ConstantInt::getBool(*context.llvmContext, value);
}

llvm::Value *LiteralIntegerAST::generate(EmissionContext &context) const {
  return llvm::ConstantInt::get(*context.llvmContext, llvm::APInt(32, this->value));
}

llvm::Value *LiteralStringAST::generate(EmissionContext &context) const {
  // FIXME: Strings are not always size 2
  const auto charType = llvm::IntegerType::getInt8Ty(*context.llvmContext);
  const auto stringType = llvm::ArrayType::get(charType, 2);

  std::vector<llvm::Constant *> values;
  values.push_back(llvm::ConstantInt::get(charType, (uint64_t)this->value.at(0)));
  values.push_back(llvm::ConstantInt::get(charType, 0));
  auto initializer = llvm::ConstantArray::get(stringType, values);

  const auto isConstant = true;
  auto globalVariable = new llvm::GlobalVariable(*context.module, stringType, isConstant, llvm::GlobalValue::PrivateLinkage,
                                  initializer, ".string.literal");
  globalVariable->setUnnamedAddr(llvm::GlobalVariable::UnnamedAddr::Global);
  globalVariable->setAlignment(llvm::MaybeAlign(1));
  return globalVariable;
}

llvm::Value *VariableDeclarationAST::generate(EmissionContext &context) const {
  auto function = context.builder->GetInsertBlock()->getParent();

  llvm::IRBuilder<> entryBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());
  auto alloca = entryBuilder.CreateAlloca(llvm::Type::getInt32Ty(*context.llvmContext), nullptr, this->varName);
  context.addVariable(this->varName, {alloca, this->isMutable});

  assert(this->value);
  auto initialValue = this->value->generate(context);
  context.builder->CreateStore(initialValue, alloca);

  return alloca;
}

llvm::Value *VariableAssignmentAST::generate(EmissionContext &context) const {
  auto lhs = context.getVariable(this->varName);
  assert(lhs.isMutable);

  auto rhs = this->value->generate(context);

  return context.builder->CreateStore(rhs, lhs.value);
}

llvm::Value *VariableReferenceAST::generate(EmissionContext &context) const {
  auto variable = context.getVariable(this->varName);
  return context.builder->CreateLoad(variable.value);
}

llvm::Value *FunctionAST::generate(EmissionContext &context) const {
  auto scopeGuard = context.pushScope();

  const auto boolType = llvm::Type::getInt1Ty(*context.llvmContext);
  const auto i32Type = llvm::Type::getInt32Ty(*context.llvmContext);

  std::vector<llvm::Type *> argumentTypes;
  for (auto argument : arguments) {
    auto argumentType = std::get<1>(argument);
    if (argumentType == "i32") {
      argumentTypes.emplace_back(i32Type);
    } else {
      assert(argumentType == "bool");
      argumentTypes.emplace_back(boolType);
    }
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
  auto argumentTypesIterator = argumentTypes.cbegin();

  // TODO: Createa a better "Type" type

  while (functionArgumentIterator != function->arg_end()) {
    assert(argumentIterator != this->arguments.cend());
    assert(argumentTypesIterator != argumentTypes.cend());

    auto &functionArgument = (*functionArgumentIterator);
    auto argumentName = std::get<0>(*argumentIterator);
    functionArgument.setName(argumentName);

    // TODO: Could we use a VariableRefAST instead of copying the code?
    auto alloca = context.builder->CreateAlloca(*argumentTypesIterator, nullptr, argumentName);
    context.builder->CreateStore(&functionArgument, alloca);
    context.addVariable(argumentName, {alloca, false});

    ++functionArgumentIterator;
    ++argumentIterator;
    ++argumentTypesIterator;
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

llvm::Value *ImportAST::generate(EmissionContext &context) const {
  // FIXME: This is a blatant cheap copy of FunctionAST, which is not really maintainable
  assert(this->name == "print");
  // TODO: We probably don't need this scope guard here
  auto scopeGuard = context.pushScope();

  // FIXME: And this is the same problem as in Literal String AST, strings are not always size 2
  const auto charType = llvm::IntegerType::getInt8Ty(*context.llvmContext);
  const auto stringType = llvm::ArrayType::get(charType, 2);
  const auto stringPointerType = llvm::PointerType::get(charType, 0);
  const auto i32Type = llvm::Type::getInt32Ty(*context.llvmContext);

  std::vector<llvm::Type *> argumentTypes;
  argumentTypes.push_back(stringType);

  llvm::Type *voidType = llvm::Type::getVoidTy(*context.llvmContext);

  auto functionType = llvm::FunctionType::get(voidType, argumentTypes, false);
  auto functionName = this->name;
  auto function =
      llvm::Function::Create(functionType, llvm::Function::InternalLinkage, functionName, context.module.get());
  context.addFunction(functionName, function);

  auto entryBlock = llvm::BasicBlock::Create(*context.llvmContext, "entry", function);
  context.builder->SetInsertPoint(entryBlock);

  auto argument = function->getArg(0);
  auto textVariable = context.builder->CreateAlloca(stringPointerType, nullptr, "text");
  context.builder->CreateStore(argument, textVariable);

  // FIXME: Do the syscall
  // call i32 asm sideeffect "syscall", "={ax},0,{di},{si},{dx},~{rcx},~{r11},~{memory},~{dirflag},~{fpsr},~{flags}"(i32
  // 1, i32 1, i8* %0, i32 1) #1, !srcloc !2

  std::vector<llvm::Type *> syscallArgumentTypes;
  syscallArgumentTypes.push_back(i32Type);
  syscallArgumentTypes.push_back(i32Type);
  syscallArgumentTypes.push_back(stringPointerType);
  syscallArgumentTypes.push_back(i32Type);

  const auto syscallFunctionType = llvm::FunctionType::get(i32Type, syscallArgumentTypes, false);
  const auto asmString = "syscall";
  const auto constraints = "={ax},0,{di},{si},{dx},~{rcx},~{r11},~{memory},~{dirflag},~{fpsr},~{flags}";
  const auto hasSideEffects = true;
  llvm::InlineAsm *assemblyCall = llvm::InlineAsm::get(syscallFunctionType, asmString, constraints, hasSideEffects);

  auto one = llvm::ConstantInt::get(*context.llvmContext, llvm::APInt(32, 1));
  std::vector<llvm::Value *> argumentValues = {one, one, argument, one};

  auto result = context.builder->CreateCall(assemblyCall, argumentValues);
  result->addAttribute(llvm::AttributeList::FunctionIndex, llvm::Attribute::NoUnwind);

  context.builder->CreateRetVoid();
  context.runPasses(function);

  return function;
}

llvm::Value *IfAST::generate(EmissionContext &context) const {
  auto &builder = context.builder;
  auto conditionValue = this->condition->generate(context);

  auto function = builder->GetInsertBlock()->getParent();
  auto thenBlock = llvm::BasicBlock::Create(*context.llvmContext, "then", function);
  auto postBlock = llvm::BasicBlock::Create(*context.llvmContext, "postIf");

  builder->CreateCondBr(conditionValue, thenBlock, postBlock);

  builder->SetInsertPoint(thenBlock);
  for (const auto &statement : this->body) {
    statement->generate(context);
  }

  if (body.back()->isTerminator() == false) {
    builder->CreateBr(postBlock);
  }

  function->getBasicBlockList().push_back(postBlock);
  builder->SetInsertPoint(postBlock);

  return llvm::UndefValue::get(llvm::Type::getVoidTy(*context.llvmContext));

  //  auto phi = builder->CreatePHI(llvm::Type::getInt32Ty(*context.llvmContext), 2, "phi");
  //  phi->addIncoming(bodyValues.back(), thenBlock);
  //  return phi;
}

llvm::Value *WhileAST::generate(EmissionContext &context) const {
  auto &builder = context.builder;

  auto function = builder->GetInsertBlock()->getParent();
  auto conditionBlock = llvm::BasicBlock::Create(*context.llvmContext, "loop.condition");
  builder->CreateBr(conditionBlock);
  function->getBasicBlockList().push_back(conditionBlock);

  auto bodyBlock = llvm::BasicBlock::Create(*context.llvmContext, "loop.body", function);
  auto exitBlock = llvm::BasicBlock::Create(*context.llvmContext, "loop.exit", function);

  builder->SetInsertPoint(conditionBlock);
  auto conditionValue = this->condition->generate(context);
  builder->CreateCondBr(conditionValue, bodyBlock, exitBlock);

  builder->SetInsertPoint(bodyBlock);
  for (const auto &statement : this->body) {
    statement->generate(context);
  }
  builder->CreateBr(conditionBlock);

  builder->SetInsertPoint(exitBlock);

  return llvm::UndefValue::get(llvm::Type::getVoidTy(*context.llvmContext));
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

llvm::Value *EqualsOperationAST::generate(EmissionContext &context) const {
  auto leftValue = this->left->generate(context);
  auto rightValue = this->right->generate(context);
  return context.builder->CreateICmpEQ(leftValue, rightValue);
}

llvm::Value *LessThanOperationAST::generate(EmissionContext &context) const {
  auto leftValue = this->left->generate(context);
  auto rightValue = this->right->generate(context);
  return context.builder->CreateICmpSLT(leftValue, rightValue);
}

llvm::Value *LessThanOrEqualToOperationAST::generate(EmissionContext &context) const {
  auto leftValue = this->left->generate(context);
  auto rightValue = this->right->generate(context);
  return context.builder->CreateICmpSLE(leftValue, rightValue);
}
