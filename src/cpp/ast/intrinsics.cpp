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

llvm::Value *PrintSyscallIntrinsicAST::generate(EmissionContext &context) const {
  const auto charType = llvm::IntegerType::getInt8Ty(*context.llvmContext);
  const auto stringPointerType = llvm::PointerType::get(charType, 0);
  const auto i32Type = context.types.i32();

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
  const auto WRITE = one;
  const auto STDOUT = one;
  auto bytecount = one;
  std::vector<llvm::Value *> argumentValues = {WRITE, STDOUT, this->text, bytecount};

  auto result = context.builder->CreateCall(assemblyCall, argumentValues);
  result->addAttribute(llvm::AttributeList::FunctionIndex, llvm::Attribute::NoUnwind);

  return result;
}

std::vector<std::tuple<std::string, std::string>> createPrintArguments(EmissionContext &) {
  // FIXME:DELETE THIS stringPointer thing
    return {{"text", "temporarystringpointer"}};
}

std::vector<std::shared_ptr<const ExpressionAST>> createPrintBody(EmissionContext &) {
  return std::vector<std::shared_ptr<const ExpressionAST>>();
}

PrintFunctionIntrinsicAST::PrintFunctionIntrinsicAST(EmissionContext &context)
    : FunctionAST("print", "i32", createPrintArguments(context), createPrintBody(context)) {

}
