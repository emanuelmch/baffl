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

#pragma once

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>

#include <memory>

struct TypeManager {

public:
  explicit TypeManager(std::shared_ptr<llvm::LLVMContext> llvmContext) : llvmContext(std::move(llvmContext)) {}

  // TODO: Return a Type of our own instead of LLVM's
  inline llvm::IntegerType *boolean() { return llvm::Type::getInt1Ty(*llvmContext); }
  inline llvm::IntegerType *i32() { return llvm::Type::getInt32Ty(*llvmContext); }
  inline llvm::IntegerType *i64() { return llvm::Type::getInt64Ty(*llvmContext); }
  inline llvm::IntegerType *character() { return llvm::Type::getInt8Ty(*llvmContext); }
  inline llvm::PointerType *string() { return llvm::PointerType::get(character(), 0); }
  inline llvm::ArrayType *string(uint64_t size) { return llvm::ArrayType::get(character(), size); }

private:
  std::shared_ptr<llvm::LLVMContext> llvmContext;
};
