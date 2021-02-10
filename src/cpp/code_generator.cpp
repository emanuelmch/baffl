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

#include "code_generator.h"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>

#include <iostream>

void initializeLlvmGenerator() {
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();
}

inline std::unique_ptr<llvm::Module> generateModule(llvm::LLVMContext &context) {
  llvm::IRBuilder<> builder(context);
  auto module = std::make_unique<llvm::Module>("baffl", context);
  llvm::legacy::FunctionPassManager passManager(module.get());
  passManager.doInitialization();

  {
    auto functionType = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), false);
    auto function = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, "main", module.get());

    auto entryBlock = llvm::BasicBlock::Create(context, "entry", function);
    builder.SetInsertPoint(entryBlock);

    auto retVal = llvm::ConstantInt::get(context, llvm::APInt(32, 0));
    builder.CreateRet(retVal);
    llvm::verifyFunction(*function);
    passManager.run(*function);
  }

//  module->print(llvm::errs(), nullptr);
  return module;
}

inline int writeModuleToFile(const std::string &output, std::unique_ptr<llvm::Module> &module) {
  // Initialize the target registry etc.
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();

  auto targetTriple = llvm::sys::getDefaultTargetTriple();
  module->setTargetTriple(targetTriple);

  std::string error;
  auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
  if (!target) {
    std::cerr << error << std::endl;
    return 1;
  }

  auto cpu = "generic";
  auto features = "";
  llvm::TargetOptions opt;
  llvm::Optional<llvm::Reloc::Model> RM;
  auto targetMachine = target->createTargetMachine(targetTriple, cpu, features, opt, RM);

  module->setDataLayout(targetMachine->createDataLayout());

  std::error_code EC;
  llvm::raw_fd_ostream dest(output, EC, llvm::sys::fs::OF_None);

  if (EC) {
    std::cout << "Could not open file: " << EC.message();
    return 1;
  }

  llvm::legacy::PassManager pass;

  if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, llvm::CGFT_ObjectFile)) {
    std::cout << "TheTargetMachine can't emit a file of this type" << std::endl;
    return 1;
  }

  pass.run(*module);
  dest.flush();

  return 0;
}

int generateObjectFile(const std::string &, const std::string &output) {
  llvm::LLVMContext context;
  std::unique_ptr<llvm::Module> module = generateModule(context);
  return writeModuleToFile(output, module);
}
