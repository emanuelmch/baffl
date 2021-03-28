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

inline static std::shared_ptr<ExpressionAST> readExpression(std::queue<Token> *tokens) {
  std::shared_ptr<ExpressionAST> expression;

  if (tokens->front().id() == literal_integer) {
    expression = std::make_shared<LiteralIntegerAST>(tokens->front().valueAsInt());
    tokens->pop();
  } else {
    assert(tokens->front().id() == name);
    // We can't tell yet whether it's a variable or a function call
    auto thingName = tokens->front().valueAsString();
    tokens->pop();

    if (tokens->front().id() == bracket_open) {
      tokens->pop();
      assert(tokens->front().id() == bracket_close);
      tokens->pop();
      expression = std::make_shared<FunctionCallAST>(thingName);
    } else {
      expression = std::make_shared<VariableReferenceAST>(thingName);
    }
    assert(tokens->front().id() == semicolon);
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
  default:
    assert(!"Statement starting with invalid token!");
  }

  assert(tokens->front().id() == semicolon);
  tokens->pop();

  return statement;
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

  assert(tokens->empty());

  return body;
}

inline std::shared_ptr<TopLevelAST> readTopLevel(std::queue<Token> *tokens) {
  // FIXME: make this generic
  assert(tokens->front().id() == keyword_function);
  tokens->pop();

  assert(tokens->front().id() == TokenType::name);
  auto name = tokens->front().valueAsString();
  tokens->pop();

  assert(tokens->front().id() == bracket_open);
  tokens->pop();

  assert(tokens->front().id() == bracket_close);
  tokens->pop();

  assert(tokens->front().id() == colon);
  tokens->pop();

  assert(tokens->front().id() == TokenType::name);
  auto returnType = tokens->front().valueAsString();
  tokens->pop();

  auto body = readBody(tokens);
  return std::make_shared<FunctionAST>(name, returnType, body);
}

std::vector<std::shared_ptr<TopLevelAST>> CodeParser::parseTopLevelExpressions(std::queue<Token> input) {
  std::vector<std::shared_ptr<TopLevelAST>> result;
  while (!input.empty()) {
    auto next = readTopLevel(&input);
    result.push_back(next);
  }
  return result;
}
