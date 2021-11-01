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

#include "emission_context.h"

#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/Utils.h> // llvm::createPromoteMemoryToRegisterPass

void Scope::addVariable(const std::string &name, const VariableReference &alloca) {
  assert(variables.count(name) == 0);
  variables.emplace(name, alloca);
}

const VariableReference &Scope::getVariable(const std::string &name) {
  if (variables.find(name) != variables.end()) {
    return variables[name];
  }

  if (parent == nullptr) {
    std::cerr << "Unknown variable: " << name << "\n";
    std::exit(1);
  }

  return parent->getVariable(name);
}
EmissionContext::EmissionContext(std::shared_ptr<llvm::LLVMContext> context)
    : llvmContext(std::move(context)), builder(std::make_shared<llvm::IRBuilder<>>(*llvmContext)),
      module(std::make_shared<llvm::Module>("baffl_main", *llvmContext)), types{llvmContext}, passManager(module.get()) {
  passManager.doInitialization();
  // Promote allocas to registers.
  passManager.add(llvm::createPromoteMemoryToRegisterPass());
  passManager.add(llvm::createLoopSimplifyPass());
}

bool EmissionContext::runPasses(llvm::Function *function) {
  auto verifyFailed = llvm::verifyFunction(*function, &llvm::errs());
  auto runFailed = passManager.run(*function);
  return verifyFailed || runFailed;
}
