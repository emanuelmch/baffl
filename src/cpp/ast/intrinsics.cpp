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

#include "./gambiarra.cpp"

struct ExtractCharFromStringAST : ExpressionAST {

  ~ExtractCharFromStringAST() override = default;

  llvm::Value *generate(EmissionContext &context) const override {
    const auto stringType = context.types.string();

    auto indexReference = VariableReferenceAST{"i"}.generate(context);
    auto textReference = VariableReferenceAST{"text"}.generate(context);

    llvm::Value *indexList[] = {indexReference};
    auto gep = context.builder->CreateGEP(stringType->getPointerElementType(), textReference, indexList);
    return VariableReferenceAST{gep, "currentChar"}.generate(context);
  }

  bool operator==(const AST &o) const override {
    auto other = dynamic_cast<const ExtractCharFromStringAST *>(&o);
    return other != nullptr;
  }
};

struct PrintConditionIntrinsicAST : public ExpressionAST {

  ~PrintConditionIntrinsicAST() override = default;

  llvm::Value *generate(EmissionContext &context) const override {
    auto currentChar = std::make_shared<ExtractCharFromStringAST>();
    auto isZero = std::make_shared<EqualsOperationAST>(currentChar, std::make_shared<LiteralIntegerAST>(0, 8));
    auto isNotZero = std::make_shared<EqualsOperationAST>(isZero, std::make_shared<LiteralBooleanAST>(false));

    return isNotZero->generate(context);
  }

  bool operator==(const AST &o) const override {
    auto other = dynamic_cast<const PrintConditionIntrinsicAST *>(&o);
    return other != nullptr;
  }
};

inline llvm::Value *generatePrintSyscall(EmissionContext &context, llvm::Value *text, llvm::Value *length) {
  const auto stringType = context.types.string();
  const auto i32Type = context.types.i32();

  std::vector<llvm::Type *> syscallArgumentTypes;
  syscallArgumentTypes.push_back(i32Type);
  syscallArgumentTypes.push_back(i32Type);
  syscallArgumentTypes.push_back(stringType);
  syscallArgumentTypes.push_back(i32Type);

  const auto syscallFunctionType = llvm::FunctionType::get(i32Type, syscallArgumentTypes, false);
  const auto asmString = "syscall";
  const auto constraints = "={ax},0,{di},{si},{dx},~{rcx},~{r11},~{memory},~{dirflag},~{fpsr},~{flags}";
  const auto hasSideEffects = true;
  llvm::InlineAsm *assemblyCall = llvm::InlineAsm::get(syscallFunctionType, asmString, constraints, hasSideEffects);

  const auto WRITE = llvm::ConstantInt::get(i32Type, 1);
  const auto STDOUT = llvm::ConstantInt::get(i32Type, 1);
  const std::vector<llvm::Value *> argumentValues = {WRITE, STDOUT, text, length};

  auto result = context.builder->CreateCall(assemblyCall, argumentValues);
  result->addAttribute(llvm::AttributeList::FunctionIndex, llvm::Attribute::NoUnwind);

  return result;
}
std::vector<std::tuple<std::string, std::string>> createPrintArguments(EmissionContext &) {
  // FIXME: DELETE THIS temporaryStringPointer thing
  return {{"text", "temporaryStringPointer"}};
}

PrintFunctionIntrinsicAST::PrintFunctionIntrinsicAST(EmissionContext &context)
    : FunctionAST("print", "i32", createPrintArguments(context), {},
                  {
                      // FunctionAttribute::Inline
                  }) {}

void PrintFunctionIntrinsicAST::generateBody(EmissionContext &context) const {
  auto zero = std::make_shared<LiteralIntegerAST>(0);
  auto index = VariableDeclarationAST{"i", zero, true}.generate(context);

  auto plus = std::make_shared<PlusOperationAST>(std::make_shared<VariableReferenceAST>("i"),
                                                 std::make_shared<LiteralIntegerAST>(1));
  auto counterIncrement = std::make_shared<VariableAssignmentAST>(index, plus);

  auto conditionAST = std::make_shared<PrintConditionIntrinsicAST>();
  WhileAST{conditionAST, {counterIncrement}}.generate(context);

  auto textReference = VariableReferenceAST{"text"}.generate(context);
  auto lengthReference = VariableReferenceAST{"i"}.generate(context);
  auto returnValue = generatePrintSyscall(context, textReference, lengthReference);

  context.builder->CreateRet(returnValue);
}

ToStringFunctionIntrinsicAST::ToStringFunctionIntrinsicAST()
    : FunctionAST("toString", "temporaryStringPointer", {{"value", "i32"}}, {},
                  {
                      // FunctionAttribute::Inline
                  }) {}

inline llvm::Value *generateMmapSyscall(EmissionContext &context, uint64_t length) {
  const auto i32Type = context.types.i32();
  // FIXME: Should be a opaque pointer instead
  const auto stringType = context.types.string();

  std::vector<llvm::Type *> syscallArgumentTypes;
  syscallArgumentTypes.push_back(i32Type);
  syscallArgumentTypes.push_back(i32Type);
  syscallArgumentTypes.push_back(i32Type);
  syscallArgumentTypes.push_back(i32Type);
  syscallArgumentTypes.push_back(i32Type);
  syscallArgumentTypes.push_back(i32Type);
  syscallArgumentTypes.push_back(i32Type);

  const auto syscallFunctionType = llvm::FunctionType::get(stringType, syscallArgumentTypes, false);
  const auto asmString = "syscall";
  const auto constraintz = "={ax},0,{di},{si},{dx},~{rcx},~{r11},~{memory},~{dirflag},~{fpsr},~{flags}";
  const auto constraints = "={ax},0,{di},{si},{dx},{r10},{r8},{r9},~{rcx},~{r11},~{memory},~{dirflag},~{fpsr},~{flags}";
  const auto hasSideEffects = true;

  //  const auto x = myVerify(syscallFunctionType, constraints);
  //  std::cerr << "success?? " << (x ? "TRUE" : "FALSE") << std::endl<< std::endl;

  llvm::InlineAsm *assemblyCall = llvm::InlineAsm::get(syscallFunctionType, asmString, constraints, hasSideEffects);

  const auto MMAP = llvm::ConstantInt::get(i32Type, 9);
  const auto addr = llvm::ConstantInt::get(i32Type, 0);
  const auto fd = llvm::ConstantInt::getSigned(i32Type, -1);
  const auto off = llvm::ConstantInt::get(i32Type, 0);

  const auto len = llvm::ConstantInt::get(i32Type, length);

  const auto PROT_READ = 0x1;
  const auto PROT_WRITE = 0x2;
  const auto prot = llvm::ConstantInt::get(i32Type, PROT_READ | PROT_WRITE);

  // TODO: We might want to use MAP_SHARED instead of MAP_ANONYMOUS in the future
  // const auto MAP_SHARED = 0x01;
  const auto MAP_PRIVATE = 0x02;
  const auto MAP_ANONYMOUS = 0x20;
  const auto flags = llvm::ConstantInt::get(i32Type, MAP_PRIVATE | MAP_ANONYMOUS);

  const std::vector<llvm::Value *> argumentValues = {MMAP, addr, len, prot, flags, fd, off};

  auto result = context.builder->CreateCall(assemblyCall, argumentValues);
  result->addAttribute(llvm::AttributeList::FunctionIndex, llvm::Attribute::NoUnwind);

  return result;
}

// FIXME: Now do it without memcpy! And actually use the value received...
void ToStringFunctionIntrinsicAST::generateBody(EmissionContext &context) const {
  // TODO: What about munmap?
  const auto charType = context.types.character();
  // auto value = VariableReferenceAST{"value"}.generate(context);

  auto newString = generateMmapSyscall(context, 11);

  auto result = std::make_shared<LiteralStringAST>("123456")->generate(context);

  const auto defaultAlign = llvm::MaybeAlign();
  context.builder->CreateMemCpy(newString, defaultAlign, result, defaultAlign, 11);
  context.builder->CreateRet(newString);
}
