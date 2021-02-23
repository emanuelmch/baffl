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

std::shared_ptr<const ExpressionAST> readExpression(std::queue<Token> *tokens) {
  //  while (!tokens->empty()) tokens->pop();
  //  return std::shared_ptr<ExpressionAST>(nullptr);
  // FIXME: make this generic
  assert(tokens->front().id() == curly_open);
  tokens->pop();

  assert(tokens->front().id() == keyword_return);
  tokens->pop();

  assert(tokens->front().id() == literal_integer);
  auto returnValueToken = tokens->front();
  auto returnValue = std::make_shared<LiteralIntegerAST>(returnValueToken.valueAsInt());
  tokens->pop();

  assert(tokens->front().id() == semicolon);
  tokens->pop();

  assert(tokens->front().id() == curly_close);
  tokens->pop();

  assert(tokens->empty());

  return std::make_shared<ReturnAST>(returnValue);
}

inline std::shared_ptr<TopLevelAST> readTopLevel(std::queue<Token> *tokens) {
  // FIXME: make this generic
  assert(tokens->front().id() == keyword_function);
  tokens->pop();

  assert(tokens->front().id() == TokenType::name);
  auto name = tokens->front().value();
  tokens->pop();

  assert(tokens->front().id() == bracket_open);
  tokens->pop();

  assert(tokens->front().id() == bracket_close);
  tokens->pop();

  assert(tokens->front().id() == colon);
  tokens->pop();

  assert(tokens->front().id() == TokenType::name);
  auto returnType = tokens->front().value();
  tokens->pop();

  auto expression = readExpression(tokens);
  return std::make_shared<FunctionAST>(name, returnType, expression);
}

std::vector<std::shared_ptr<TopLevelAST>> CodeParser::parseTopLevelExpressions(std::queue<Token> input) {
  std::vector<std::shared_ptr<TopLevelAST>> result;
  while (!input.empty()) {
    auto next = readTopLevel(&input);
    result.push_back(next);
  }
  return result;
}
