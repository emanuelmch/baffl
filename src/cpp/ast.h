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

#pragma once

#include "ast/emission_context.h"
#include "ast/function_attributes.h"
#include "code_lexer.h"

#include <llvm/IR/Value.h>

#include <unordered_set>
#include <utility>

// Interfaces
struct AST {
  virtual llvm::Value *generate(EmissionContext &) const = 0;

  virtual bool operator==(const AST &) const = 0;
};

struct TopLevelAST : public AST {
  virtual ~TopLevelAST() = default;
};

struct ExpressionAST : public AST {
  virtual ~ExpressionAST() = default;

  [[nodiscard]] virtual bool isTerminator() const { return false; }
};

// Generic Helpers
inline static bool compareBodies(const std::vector<std::shared_ptr<const ExpressionAST>> &left,
                                 const std::vector<std::shared_ptr<const ExpressionAST>> &right) {
  if (left.size() != right.size()) return false;

  for (auto l = left.cbegin(), r = right.cbegin(); l != left.cend(); ++l, ++r) {
    auto leftItem = l->get();
    auto rightItem = r->get();

    // TODO: Replace this with `*leftItem != *rightItem` on C++20
    if (!leftItem || !leftItem->operator==(*rightItem)) {
      return false;
    }
  }

  return true;
}

// Literals
struct LiteralBooleanAST : public ExpressionAST {
  const bool value;

  explicit LiteralBooleanAST(bool value) : value(value) {}
  ~LiteralBooleanAST() override = default;

  llvm::Value *generate(EmissionContext &) const override;

  inline bool operator==(const AST &o) const override {
    auto other = dynamic_cast<const LiteralBooleanAST *>(&o);
    return other && this->value == other->value;
  }
};
struct LiteralIntegerAST : public ExpressionAST {
  const uint64_t value;
  const uint8_t length;

  explicit LiteralIntegerAST(uint64_t value) : value{value}, length{32} {}
  explicit LiteralIntegerAST(uint64_t value, uint8_t length) : value{value}, length{length} {}
  ~LiteralIntegerAST() override = default;

  llvm::Value *generate(EmissionContext &) const override;

  inline bool operator==(const AST &o) const override {
    auto other = dynamic_cast<const LiteralIntegerAST *>(&o);
    return other && this->value == other->value;
  }
};
struct LiteralStringAST : public ExpressionAST {
  const std::string value;

  explicit LiteralStringAST(std::string value) : value(std::move(value)) {}
  ~LiteralStringAST() override = default;

  llvm::Value *generate(EmissionContext &) const override;

  inline bool operator==(const AST &o) const override {
    auto other = dynamic_cast<const LiteralStringAST *>(&o);
    return other && this->value == other->value;
  }
};

// Variables
struct VariableDeclarationAST : public ExpressionAST {
  const std::string varName;
  const std::shared_ptr<ExpressionAST> value;
  const bool isMutable;

  VariableDeclarationAST(std::string varName, std::shared_ptr<ExpressionAST> value, bool isMutable = false)
      : varName(std::move(varName)), value(std::move(value)), isMutable(isMutable) {}

  llvm::Value *generate(EmissionContext &) const override;

  inline bool operator==(const AST &o) const override {
    auto other = dynamic_cast<const VariableDeclarationAST *>(&o);
    return other && this->varName == other->varName && *(this->value) == *(other->value) &&
           this->isMutable == other->isMutable;
  }
};

struct VariableAssignmentAST : public ExpressionAST {
  llvm::Value *const variable;
  const std::string varName;
  const std::shared_ptr<ExpressionAST> value;

  VariableAssignmentAST(llvm::Value *variable, std::shared_ptr<ExpressionAST> value)
      : variable{variable}, varName{}, value{std::move(value)} {}
  VariableAssignmentAST(std::string varName, std::shared_ptr<ExpressionAST> value)
      : variable{nullptr}, varName{std::move(varName)}, value{std::move(value)} {}

  llvm::Value *generate(EmissionContext &) const override;

  inline bool operator==(const AST &o) const override {
    auto other = dynamic_cast<const VariableAssignmentAST *>(&o);
    return other && this->varName == other->varName && *(this->value) == *(other->value);
  }
};

struct VariableReferenceAST : public ExpressionAST {
  llvm::Value *const variable;
  const std::string varName;
  const std::string loadName;

  explicit VariableReferenceAST(llvm::Value *variable, std::string loadName = {})
      : variable{variable}, varName{}, loadName{std::move(loadName)} {}
  explicit VariableReferenceAST(std::string varName, std::string loadName = {})
      : variable{nullptr}, varName{std::move(varName)}, loadName{std::move(loadName)} {}

  llvm::Value *generate(EmissionContext &) const override;

  inline bool operator==(const AST &o) const override {
    auto other = dynamic_cast<const VariableReferenceAST *>(&o);
    return other && this->varName == other->varName;
  }
};

// Functions
struct FunctionAST : public TopLevelAST {
  const std::string name;
  // TODO: Change this to a type reference
  const std::string returnTypeName;
  // TODO: Delete fakeArguments and rename the real ones to just "arguments"
  const std::vector<VariableReference> realArguments;
  const std::vector<std::tuple<std::string, std::string>> fakeArguments;
  const std::vector<std::shared_ptr<const ExpressionAST>> body;
  const std::unordered_set<FunctionAttribute> attributes;

  FunctionAST(std::string name, std::string returnTypeName, std::vector<VariableReference> arguments,
              std::vector<std::shared_ptr<const ExpressionAST>> body)
      : name{std::move(name)}, returnTypeName{std::move(returnTypeName)}, realArguments{std::move(arguments)},
        fakeArguments{}, body{std::move(body)} {}

  [[deprecated("Use the version that takes VariableReference instead")]] FunctionAST(
      std::string name, std::string returnTypeName, std::vector<std::tuple<std::string, std::string>> arguments,
      std::vector<std::shared_ptr<const ExpressionAST>> body, std::unordered_set<FunctionAttribute> attributes = {})
      : name{std::move(name)}, returnTypeName{std::move(returnTypeName)}, realArguments{},
        fakeArguments{std::move(arguments)}, body{std::move(body)}, attributes{std::move(attributes)} {}
  ~FunctionAST() override = default;

  llvm::Value *generate(EmissionContext &) const override;

  virtual void generateBody(EmissionContext &) const; // Overridden in Intrinsic functions

  inline bool operator==(const AST &o) const override {
    auto other = dynamic_cast<const FunctionAST *>(&o);
    return other && this->name == other->name && this->returnTypeName == other->returnTypeName &&
           compareBodies(this->body, other->body);
  }

private:
  // TODO: Delete this function after we delete the fakeArguments
  std::vector<VariableReference> generateRealArguments(llvm::Function *, const std::vector<llvm::Type *> &) const;
};

struct FunctionCallAST : public ExpressionAST {
  const std::string functionName;
  const std::vector<std::shared_ptr<ExpressionAST>> arguments;

  explicit FunctionCallAST(std::string functionName, std::vector<std::shared_ptr<ExpressionAST>> arguments = {})
      : functionName(std::move(functionName)), arguments(std::move(arguments)) {}
  ~FunctionCallAST() override = default;

  llvm::Value *generate(EmissionContext &context) const override;

  bool operator==(const AST &o) const override {
    auto other = dynamic_cast<const FunctionCallAST *>(&o);
    return other && this->functionName == other->functionName;
  }
};

// Imports
struct ImportAST : public TopLevelAST {
  const std::string name;

  explicit ImportAST(std::string name) : name(std::move(name)) {}
  ~ImportAST() override = default;

  llvm::Value *generate(EmissionContext &) const override;

  inline bool operator==(const AST &o) const override {
    auto other = dynamic_cast<const ImportAST *>(&o);
    return other && this->name == other->name;
  }
};

// Conditions
struct IfAST : public ExpressionAST {
  const std::shared_ptr<const ExpressionAST> condition;
  const std::vector<std::shared_ptr<const ExpressionAST>> body;

  IfAST(std::shared_ptr<const ExpressionAST> condition, std::vector<std::shared_ptr<const ExpressionAST>> body)
      : condition(std::move(condition)), body(std::move(body)) {}
  ~IfAST() override = default;

  llvm::Value *generate(EmissionContext &context) const override;
  inline bool operator==(const AST &o) const override {
    auto other = dynamic_cast<const IfAST *>(&o);
    return other && *(this->condition) == *(other->condition) && compareBodies(this->body, other->body);
  }
};

// Loops
struct WhileAST : public ExpressionAST {
  const std::shared_ptr<const ExpressionAST> condition;
  const std::vector<std::shared_ptr<const ExpressionAST>> body;

  WhileAST(std::shared_ptr<const ExpressionAST> condition, std::vector<std::shared_ptr<const ExpressionAST>> body)
      : condition(std::move(condition)), body(std::move(body)) {}
  ~WhileAST() override = default;

  llvm::Value *generate(EmissionContext &context) const override;
  inline bool operator==(const AST &o) const override {
    auto other = dynamic_cast<const WhileAST *>(&o);
    return other && *(this->condition) == *(other->condition) && compareBodies(this->body, other->body);
  }
};

// Operations
struct ReturnAST : public ExpressionAST {
  const std::shared_ptr<const ExpressionAST> value;

  explicit ReturnAST(std::shared_ptr<const ExpressionAST> value) : value(std::move(value)) {}
  ~ReturnAST() override = default;

  llvm::Value *generate(EmissionContext &) const override;

  inline bool operator==(const AST &o) const override {
    auto other = dynamic_cast<const ReturnAST *>(&o);
    return other && *(this->value) == *(other->value);
  }

  [[nodiscard]] bool isTerminator() const override { return true; }
};

// TODO: Should we replace all these with a "BinaryOperationAST"?
struct PlusOperationAST : public ExpressionAST {
  const std::shared_ptr<const ExpressionAST> left;
  const std::shared_ptr<const ExpressionAST> right;

  PlusOperationAST(std::shared_ptr<const ExpressionAST> left, std::shared_ptr<const ExpressionAST> right)
      : left(std::move(left)), right(std::move(right)) {}

  llvm::Value *generate(EmissionContext &) const override;

  inline bool operator==(const AST &o) const override {
    auto other = dynamic_cast<const PlusOperationAST *>(&o);
    return other && *(this->left) == *(other->left) && *(this->right) == *(other->right);
  }
};

struct MinusOperationAST : public ExpressionAST {
  const std::shared_ptr<const ExpressionAST> left;
  const std::shared_ptr<const ExpressionAST> right;

  MinusOperationAST(std::shared_ptr<const ExpressionAST> left, std::shared_ptr<const ExpressionAST> right)
      : left(std::move(left)), right(std::move(right)) {}

  llvm::Value *generate(EmissionContext &) const override;

  inline bool operator==(const AST &o) const override {
    auto other = dynamic_cast<const MinusOperationAST *>(&o);
    return other && *(this->left) == *(other->left) && *(this->right) == *(other->right);
  }
};

struct DivisionOperationAST : public ExpressionAST {
  const std::shared_ptr<const ExpressionAST> left;
  const std::shared_ptr<const ExpressionAST> right;

  DivisionOperationAST(std::shared_ptr<const ExpressionAST> left, std::shared_ptr<const ExpressionAST> right)
      : left(std::move(left)), right(std::move(right)) {}

  llvm::Value *generate(EmissionContext &) const override;

  inline bool operator==(const AST &o) const override {
    auto other = dynamic_cast<const DivisionOperationAST *>(&o);
    return other && *(this->left) == *(other->left) && *(this->right) == *(other->right);
  }
};

struct ModuloOperationAST : public ExpressionAST {
  const std::shared_ptr<const ExpressionAST> left;
  const std::shared_ptr<const ExpressionAST> right;

  ModuloOperationAST(std::shared_ptr<const ExpressionAST> left, std::shared_ptr<const ExpressionAST> right)
      : left(std::move(left)), right(std::move(right)) {}

  llvm::Value *generate(EmissionContext &) const override;

  inline bool operator==(const AST &o) const override {
    auto other = dynamic_cast<const ModuloOperationAST *>(&o);
    return other && *(this->left) == *(other->left) && *(this->right) == *(other->right);
  }
};

struct EqualsOperationAST : public ExpressionAST {
  const std::shared_ptr<const ExpressionAST> left;
  const std::shared_ptr<const ExpressionAST> right;

  EqualsOperationAST(std::shared_ptr<const ExpressionAST> left, std::shared_ptr<const ExpressionAST> right)
      : left(std::move(left)), right(std::move(right)) {}

  llvm::Value *generate(EmissionContext &) const override;

  inline bool operator==(const AST &o) const override {
    auto other = dynamic_cast<const EqualsOperationAST *>(&o);
    return other && *(this->left) == *(other->left) && *(this->right) == *(other->right);
  }
};

struct LessThanOperationAST : public ExpressionAST {
  const std::shared_ptr<const ExpressionAST> left;
  const std::shared_ptr<const ExpressionAST> right;

  LessThanOperationAST(std::shared_ptr<const ExpressionAST> left, std::shared_ptr<const ExpressionAST> right)
      : left(std::move(left)), right(std::move(right)) {}

  llvm::Value *generate(EmissionContext &) const override;

  inline bool operator==(const AST &o) const override {
    auto other = dynamic_cast<const LessThanOperationAST *>(&o);
    return other && *(this->left) == *(other->left) && *(this->right) == *(other->right);
  }
};

struct LessThanOrEqualToOperationAST : public ExpressionAST {
  const std::shared_ptr<const ExpressionAST> left;
  const std::shared_ptr<const ExpressionAST> right;

  LessThanOrEqualToOperationAST(std::shared_ptr<const ExpressionAST> left, std::shared_ptr<const ExpressionAST> right)
      : left(std::move(left)), right(std::move(right)) {}

  llvm::Value *generate(EmissionContext &) const override;

  inline bool operator==(const AST &o) const override {
    auto other = dynamic_cast<const LessThanOrEqualToOperationAST *>(&o);
    return other && *(this->left) == *(other->left) && *(this->right) == *(other->right);
  }
};
