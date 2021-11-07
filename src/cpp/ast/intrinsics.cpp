/*
 * Copyright (c) 2021 Emanuel Machado da Silva
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

#include "intrinsics.h"

#include <llvm/IR/InlineAsm.h>
#include <llvm/IR/Instructions.h>

llvm::Value *generatePrintSyscall(EmissionContext &context, llvm::Value *text, llvm::Value *length) {
  const auto charType = llvm::IntegerType::getInt8Ty(*context.llvmContext);
  const auto stringPointerType = llvm::PointerType::get(charType, 0);
  const auto i32Type = context.types.i32();
  const auto i64Type = context.types.i64();

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

  const auto WRITE = llvm::ConstantInt::get(i64Type, 1);
  const auto STDOUT = llvm::ConstantInt::get(i64Type, 1);
  std::vector<llvm::Value *> argumentValues = {WRITE, STDOUT, text, length};

  auto result = context.builder->CreateCall(assemblyCall, argumentValues);
  result->addAttribute(llvm::AttributeList::FunctionIndex, llvm::Attribute::NoUnwind);

  return result;
}

std::vector<std::tuple<std::string, std::string>> createPrintArguments(EmissionContext &) {
  // FIXME: DELETE THIS temporaryStringPointer thing
  return {{"text", "temporaryStringPointer"}};
}

PrintFunctionIntrinsicAST::PrintFunctionIntrinsicAST(EmissionContext &context)
    : FunctionAST("print", "void", createPrintArguments(context), {}) {}

void PrintFunctionIntrinsicAST::generateBody(EmissionContext &context) const {
  auto zero = std::make_shared<LiteralIntegerAST>(0);
  auto counter = VariableDeclarationAST{"i", zero, true}.generate(context);


  auto plus = std::make_shared<PlusOperationAST>(std::make_shared<VariableReferenceAST>("i"),
                                                 std::make_shared<LiteralIntegerAST>(1));
  auto counterIncrement = std::make_shared<VariableAssignmentAST>("i", plus);

  auto conditionAST = std::make_shared<PrintConditionIntrinsicAST>();
  WhileAST{conditionAST, {counterIncrement}}.generate(context);

  auto textReference = VariableReferenceAST{"text"}.generate(context);
  auto lengthReference = VariableReferenceAST{"i"}.generate(context);
  generatePrintSyscall(context, textReference, lengthReference);
}

struct ExtractElementAST : ExpressionAST {

  ~ExtractElementAST() override = default;

  llvm::Value *generate(EmissionContext &context) const override {
    const auto i64Type = context.types.i64();
    const auto charType = context.types.character();
    const auto stringType = context.types.string();

//    auto textReference = context.getVariable("text").value;
    //    auto indexReference = context.getVariable("i").value;
    auto indexReference = VariableReferenceAST{"i"}.generate(context);
        auto textReference = VariableReferenceAST{"text"}.generate(context);

    // return context.builder->CreateExtractElement(textReference, indexReference);
    const auto zero = llvm::ConstantInt::get(i64Type, 0);
//        llvm::Value *indexList[] = {zero, indexReference};
    llvm::Value *indexList[] = {indexReference};
    //    const auto inbounds = true;
    //    return llvm::ConstantExpr::getGetElementPtr(charType, textReference, indexList, inbounds);
//    auto gep = llvm::GetElementPtrInst::CreateInBounds(charType, textReference, indexList);
    auto gep = context.builder->CreateGEP(textReference, indexList);
//    auto gep2 = llvm::GetElementPtrInst::CreateInBounds(stringType, gep, indexList);
    return context.builder->CreateLoad(charType, gep, "currentChar");
  }

  bool operator==(const AST &o) const override {
    auto other = dynamic_cast<const ExtractElementAST *>(&o);
    return other != nullptr;
  }
};

llvm::Value *PrintConditionIntrinsicAST::generate(EmissionContext &context) const {
  auto currentChar = std::make_shared<ExtractElementAST>();
  auto isZero = std::make_shared<EqualsOperationAST>(currentChar, std::make_shared<LiteralIntegerAST>(0, 8));
  auto isNotZero = std::make_shared<EqualsOperationAST>(isZero, std::make_shared<LiteralBooleanAST>(false));

  return isNotZero->generate(context);
}
