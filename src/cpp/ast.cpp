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

#include "ast/intrinsics.h"

#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InlineAsm.h>

llvm::Value *LiteralBooleanAST::generate(EmissionContext &context) const {
  return llvm::ConstantInt::getBool(*context.llvmContext, value);
}

llvm::Value *LiteralIntegerAST::generate(EmissionContext &context) const {
  return llvm::ConstantInt::get(*context.llvmContext, llvm::APInt(length, this->value));
}

llvm::Value *LiteralStringAST::generate(EmissionContext &context) const {
  const auto i64Type = context.types.i64();
  const auto charType = context.types.character();
  const auto stringType = context.types.string(this->value.size());

  // TODO: Replace all usages of `ConstantInt::get(type` with `type.getConstant`
  std::vector<llvm::Constant *> values;
  for (auto c : this->value) {
    values.push_back(llvm::ConstantInt::get(charType, (uint8_t)c));
  }
  values.push_back(llvm::ConstantInt::get(charType, 0));
  auto initializer = llvm::ConstantArray::get(stringType, values);

  const auto isConstant = true;
  auto globalVariable = new llvm::GlobalVariable(*context.module, stringType, isConstant,
                                                 llvm::GlobalValue::PrivateLinkage, initializer, ".string.literal");
  globalVariable->setUnnamedAddr(llvm::GlobalVariable::UnnamedAddr::Global);
  globalVariable->setAlignment(llvm::MaybeAlign(1));

  llvm::Value *indexList[] = {llvm::ConstantInt::get(i64Type, 0), llvm::ConstantInt::get(i64Type, 0)};
  const auto inbounds = true;
  return llvm::ConstantExpr::getGetElementPtr(stringType, globalVariable, indexList, inbounds);
}

llvm::Value *VariableDeclarationAST::generate(EmissionContext &context) const {
  auto function = context.builder->GetInsertBlock()->getParent();

  assert(this->value);
  auto initialValue = this->value->generate(context);

  // TODO: Allow variable declarations to be non-inferred
  auto type = initialValue->getType();

  llvm::IRBuilder<> entryBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());
  auto alloca = entryBuilder.CreateAlloca(type, nullptr, this->varName);

  context.addVariable({varName, type, alloca, isMutable});

  context.builder->CreateStore(initialValue, alloca);

  return alloca;
}

llvm::Value *VariableAssignmentAST::generate(EmissionContext &context) const {
  llvm::Value *lhs;

  if (variable != nullptr) {
    lhs = variable;
  } else {
    auto reference = context.getVariable(this->varName);
    assert(reference.isMutable);
    lhs = reference.value;
  }

  auto rhs = this->value->generate(context);
  return context.builder->CreateStore(rhs, lhs);
}

llvm::Value *VariableReferenceAST::generate(EmissionContext &context) const {
  llvm::Value *reference;

  if (variable != nullptr) {
    reference = variable;
  } else {
    reference = context.getVariable(this->varName).value;
  }
  auto variableType = reference->getType()->getPointerElementType();

  return context.builder->CreateLoad(variableType, reference, loadName);
}

llvm::Value *FunctionAST::generate(EmissionContext &context) const {
  auto scopeGuard = context.pushScope();

  const auto boolType = llvm::Type::getInt1Ty(*context.llvmContext);
  const auto i32Type = llvm::Type::getInt32Ty(*context.llvmContext);

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

  std::vector<llvm::Type *> argumentTypes;
  for (auto argument : fakeArguments) {
    auto argumentType = std::get<1>(argument);
    if (argumentType == "i32") {
      argumentTypes.emplace_back(i32Type);
    } else if (argumentType == "temporaryStringPointer") {
      // FIXME: DELETE THIS temporaryStringPointer thing, it's used in PrintFunctionIntrinsicAST
      const auto charType = llvm::IntegerType::getInt8Ty(*context.llvmContext);
      const auto stringPointerType = llvm::PointerType::get(charType, 0);
      argumentTypes.emplace_back(stringPointerType);
    } else {
      assert(argumentType == "bool");
      argumentTypes.emplace_back(boolType);
    }
  }

  auto functionType = llvm::FunctionType::get(returnType, argumentTypes, false);
  auto functionName = this->name;
  auto function =
      llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, functionName, context.module.get());
  context.addFunction(functionName, function);

  auto entryBlock = llvm::BasicBlock::Create(*context.llvmContext, "entry", function);
  context.builder->SetInsertPoint(entryBlock);

  std::vector<VariableReference> arguments;

  if (realArguments.empty() == false) {
    arguments = realArguments;
  } else if (fakeArguments.empty() == false) {
    arguments = generateRealArguments(function, argumentTypes);
  }

  for (const auto &argument : arguments) {
    auto alloca = context.builder->CreateAlloca(argument.type, nullptr, argument.name);
    context.builder->CreateStore(argument.value, alloca);

    VariableReference ref{argument.name, argument.type, alloca, false};
    context.addVariable(ref);
  }

  this->generateBody(context);

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

std::vector<VariableReference>
FunctionAST::generateRealArguments(llvm::Function *function, const std::vector<llvm::Type *> &argumentTypes) const {
  std::vector<VariableReference> result;

  auto functionArgumentIterator = function->arg_begin();
  auto argumentIterator = this->fakeArguments.cbegin();
  auto argumentTypesIterator = argumentTypes.cbegin();

  // TODO: Create a better "Type" type
  while (functionArgumentIterator != function->arg_end()) {
    assert(argumentIterator != this->fakeArguments.cend());
    assert(argumentTypesIterator != argumentTypes.cend());

    auto &functionArgument = (*functionArgumentIterator);
    const auto &argumentName = std::get<0>(*argumentIterator);
    const auto &argumentType = *argumentTypesIterator;

    functionArgument.setName(argumentName);

    VariableReference ref{argumentName, argumentType, &functionArgument, false};
    result.push_back(ref);
    // TODO: Discover why doesn't this emplace_back work
    // result.emplace_back(argumentName, argumentType, &functionArgument, false);

    ++functionArgumentIterator;
    ++argumentIterator;
    ++argumentTypesIterator;
  }

  return result;
}

void FunctionAST::generateBody(EmissionContext &context) const {
  for (const auto &expression : body) {
    expression->generate(context);
  }
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
  assert(this->name == "print");
  return PrintFunctionIntrinsicAST(context).generate(context);
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
  if (leftValue->getType() != rightValue->getType()) {
    auto typeA = leftValue->getType()->getTypeID();
    auto typeB = rightValue->getType()->getTypeID();
    assert(typeA == typeB);
    return context.builder->CreateICmpEQ(leftValue, rightValue);
  }
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
