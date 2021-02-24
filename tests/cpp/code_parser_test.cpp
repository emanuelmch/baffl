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

#include "cpp/code_parser.h"

#include "ast_helper.h"
#include <gtest/gtest.h>

TEST(CodeLexer, Trivial) {
  auto returnValueAST = std::make_shared<LiteralIntegerAST>(0);
  auto returnExpressionAST = std::make_shared<ReturnAST>(returnValueAST);
  auto functionAST = std::make_shared<FunctionAST>("main", "i32", returnExpressionAST);

  std::vector<std::shared_ptr<TopLevelAST>> expected{functionAST};

  std::queue<Token> input;
  input.push(Token(keyword_function));
  input.push(Token(name, "main"));
  input.push(Token(bracket_open));
  input.push(Token(bracket_close));
  input.push(Token(colon));
  input.push(Token(name, "i32"));
  input.push(Token(curly_open));
  input.push(Token(keyword_return));
  input.push(Token(literal_integer, "0"));
  input.push(Token(semicolon));
  input.push(Token(curly_close));

  auto actual = CodeParser::parseTopLevelExpressions(input);

  EXPECT_EQ(expected, actual);
}

TEST(CodeLexer, Trivial_Incorrect) {
  auto returnValueAST = std::make_shared<LiteralIntegerAST>(0);
  auto returnExpressionAST = std::make_shared<ReturnAST>(returnValueAST);
  auto functionAST = std::make_shared<FunctionAST>("main", "i32", returnExpressionAST);

  std::vector<std::shared_ptr<TopLevelAST>> expected{functionAST};

  std::queue<Token> input;
  input.push(Token(keyword_function));
  input.push(Token(name, "main"));
  input.push(Token(bracket_open));
  input.push(Token(bracket_close));
  input.push(Token(colon));
  input.push(Token(name, "i32"));
  input.push(Token(curly_open));
  input.push(Token(keyword_return));
  input.push(Token(literal_integer, "1"));
  input.push(Token(semicolon));
  input.push(Token(curly_close));

  auto actual = CodeParser::parseTopLevelExpressions(input);

  EXPECT_NE(expected, actual);
}
