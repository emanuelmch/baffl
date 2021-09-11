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

#include "code_parser.h"

inline static std::shared_ptr<ExpressionAST> readExpression(std::queue<Token> *tokens);

inline static std::shared_ptr<ExpressionAST> readPrimary(std::queue<Token> *tokens) {
  std::shared_ptr<ExpressionAST> expression;

  const auto next = tokens->front();
  if (next.id() == literal_integer) {
    expression = std::make_shared<LiteralIntegerAST>(next.valueAsInt());
    tokens->pop();
  } else if (next.id() == keyword_true || next.id() == keyword_false) {
    expression = std::make_shared<LiteralBooleanAST>(next.id() == keyword_true);
    tokens->pop();
  } else {
    assert(next.id() == name);
    // We can't tell yet whether it's a variable or a function call
    auto thingName = tokens->front().valueAsString();
    tokens->pop();

    if (tokens->front().id() == bracket_open) {
      std::vector<std::shared_ptr<ExpressionAST>> arguments;

      tokens->pop();
      while (tokens->front().id() != bracket_close) {
        arguments.push_back(readExpression(tokens));
        assert(tokens->front().id() == comma || tokens->front().id() == bracket_close);
        if (tokens->front().id() == comma) {
          tokens->pop();
        }
      }
      assert(tokens->front().id() == bracket_close);
      tokens->pop();

      expression = std::make_shared<FunctionCallAST>(thingName, arguments);
    } else {
      expression = std::make_shared<VariableReferenceAST>(thingName);
    }
  }

  return expression;
}

inline static std::shared_ptr<ExpressionAST> readExpression(std::queue<Token> *tokens) {
  std::shared_ptr<ExpressionAST> expression = readPrimary(tokens);

  auto next = tokens->front().id();
  while (next != semicolon && next != bracket_close) {
    // Binary operators
    // TODO: Stop repeating the same code for every operator
    if (next == operator_plus) {
      tokens->pop();
      auto right = readPrimary(tokens);
      expression = std::make_shared<PlusOperationAST>(expression, right);
    } else if (next == operator_minus) {
      tokens->pop();
      auto right = readPrimary(tokens);
      expression = std::make_shared<MinusOperationAST>(expression, right);
    } else {
      assert(!"Unexpected token");
    }

    next = tokens->front().id();
  }

  return expression;
}

inline static std::shared_ptr<ExpressionAST> readStatement(std::queue<Token> *tokens) {
  std::shared_ptr<ExpressionAST> statement;

  switch (tokens->front().id()) {
  case keyword_return: {
    tokens->pop();
    auto returnValue = readExpression(tokens);
    statement = std::make_shared<ReturnAST>(returnValue);
    break;
  }
  case keyword_let: {
    tokens->pop();
    assert(tokens->front().id() == name);
    auto varName = tokens->front().valueAsString();
    tokens->pop();
    assert(tokens->front().id() == operator_assign);
    tokens->pop();
    auto initialValue = readExpression(tokens);
    statement = std::make_shared<VariableDeclarationAST>(varName, initialValue);
    break;
  }
  case name:
    statement = readExpression(tokens);
    assert(dynamic_cast<const FunctionCallAST *>(statement.get()));
    break;
  default:
    assert(!"Statement starting with invalid token!");
  }

  assert(tokens->front().id() == semicolon);
  tokens->pop();

  return statement;
}

inline static std::tuple<std::string, std::string> readArgument(std::queue<Token> *tokens) {
  assert(tokens->front().id() == TokenType::name);
  auto name = tokens->front().valueAsString();
  tokens->pop();

  assert(tokens->front().id() == colon);
  tokens->pop();

  assert(tokens->front().id() == TokenType::name);
  auto type = tokens->front().valueAsString();
  tokens->pop();

  return std::make_tuple(name, type);
}

inline static std::vector<std::shared_ptr<const ExpressionAST>> readBody(std::queue<Token> *tokens) {
  // FIXME: make this generic
  assert(tokens->front().id() == curly_open);
  tokens->pop();

  std::vector<std::shared_ptr<const ExpressionAST>> body;

  while (tokens->front().id() != curly_close) {
    // FIXME: Support multiple {} levels
    assert(tokens->front().id() != curly_open);

    // FIXME: Don't assume our return value will always come from the last statement
    body.push_back(readStatement(tokens));
  }
  tokens->pop();

  return body;
}

inline std::shared_ptr<TopLevelAST> readTopLevel(std::queue<Token> *tokens) {
  std::vector<std::tuple<std::string, std::string>> arguments;
  // FIXME: make this generic
  assert(tokens->front().id() == keyword_function);
  tokens->pop();

  assert(tokens->front().id() == TokenType::name);
  auto name = tokens->front().valueAsString();
  tokens->pop();

  assert(tokens->front().id() == bracket_open);
  tokens->pop();

  if (tokens->front().id() == TokenType::name) {
    arguments.push_back(readArgument(tokens));
    while (tokens->front().id() == TokenType::comma) {
      arguments.push_back(readArgument(tokens));
    }
  }

  assert(tokens->front().id() == bracket_close);
  tokens->pop();

  std::string returnType;
  if (tokens->front().id() == curly_open) {
    returnType = "void";
  } else {
    assert(tokens->front().id() == colon);
    tokens->pop();

    assert(tokens->front().id() == TokenType::name);
    returnType = tokens->front().valueAsString();
    tokens->pop();
  }

  auto body = readBody(tokens);
  return std::make_shared<FunctionAST>(name, returnType, arguments, body);
}

std::vector<std::shared_ptr<TopLevelAST>> CodeParser::parseTopLevelExpressions(std::queue<Token> input) {
  std::vector<std::shared_ptr<TopLevelAST>> result;
  while (!input.empty()) {
    auto next = readTopLevel(&input);
    result.push_back(next);
  }
  return result;
}
