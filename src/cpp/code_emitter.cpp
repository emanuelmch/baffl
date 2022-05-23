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

#include "code_emitter.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Passes/PassBuilder.h> // llvm::PassBuilder::OptimizationLevel
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>

#include <iostream>
#include <memory> // std::unique_ptr

inline std::shared_ptr<llvm::Module> generateModule(const std::shared_ptr<llvm::LLVMContext> &llvmContext,
                                                    const std::vector<std::shared_ptr<TopLevelAST>> &ast) {
  EmissionContext emissionContext(llvmContext);

  for (const auto &topLevel : ast) {
    topLevel->generate(emissionContext);
  }

  return emissionContext.module;
}

inline void runOptimizationPasses(std::shared_ptr<llvm::Module> &module,
                                  std::unique_ptr<llvm::TargetMachine> &targetMachine) {
  llvm::LoopAnalysisManager loopAnalysisManager;
  llvm::FunctionAnalysisManager functionAnalysisManager;
  llvm::CGSCCAnalysisManager cgsccAnalysisManager;
  llvm::ModuleAnalysisManager moduleAnalysisManager;

  llvm::PassBuilder passBuilder{targetMachine.get()};
  passBuilder.registerModuleAnalyses(moduleAnalysisManager);
  passBuilder.registerCGSCCAnalyses(cgsccAnalysisManager);
  passBuilder.registerFunctionAnalyses(functionAnalysisManager);
  passBuilder.registerLoopAnalyses(loopAnalysisManager);
  passBuilder.crossRegisterProxies(loopAnalysisManager, functionAnalysisManager, cgsccAnalysisManager,
                                   moduleAnalysisManager);

  passBuilder                                                                  //
      .buildPerModuleDefaultPipeline(llvm::PassBuilder::OptimizationLevel::O2) //
      .run(*module, moduleAnalysisManager);                                    //
}

inline OperationResult writeModuleToFile(const std::string &output, std::shared_ptr<llvm::Module> &module) {
  auto verifyFailed = llvm::verifyModule(*module, &llvm::errs());
  if (verifyFailed) {
    std::cerr << "Module verification failed" << std::endl;
    return FAILURE;
  }

  auto targetTriple = llvm::sys::getDefaultTargetTriple();
  module->setTargetTriple(targetTriple);

  std::string error;
  auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
  if (!target) {
    std::cerr << error << std::endl;
    return FAILURE;
  }

  auto cpu = "generic";
  auto features = "";
  llvm::TargetOptions opt;
  llvm::Optional<llvm::Reloc::Model> RM;
  std::unique_ptr<llvm::TargetMachine> targetMachine{target->createTargetMachine(targetTriple, cpu, features, opt, RM)};

  module->setDataLayout(targetMachine->createDataLayout());

  std::error_code errorCode;
  llvm::raw_fd_ostream dest(output, errorCode, llvm::sys::fs::OF_None);

  if (errorCode) {
    std::cout << "Could not open file: " << errorCode.message();
    return FAILURE;
  }

  runOptimizationPasses(module, targetMachine);

  {
    llvm::legacy::PassManager objectEmissionPassManager;
    if (targetMachine->addPassesToEmitFile(objectEmissionPassManager, dest, nullptr, llvm::CGFT_ObjectFile)) {
      std::cout << "TheTargetMachine can't emit a file of this type" << std::endl;
      return FAILURE;
    }
    objectEmissionPassManager.run(*module);
  }

  dest.flush();

  return SUCCESS;
}

OperationResult CodeEmitter::emitObjectFile(const std::vector<std::shared_ptr<TopLevelAST>> &topLevel,
                                            const std::string &outputFile, bool isVerbose) {
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();

  auto context = std::make_shared<llvm::LLVMContext>();
  auto module = generateModule(context, topLevel);

  auto result = writeModuleToFile(outputFile, module);

  if (result == SUCCESS && isVerbose) {
    module->print(llvm::outs(), nullptr);
  }

  return result;
}
