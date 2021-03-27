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

#include "test_helpers/ast.h"
#include "test_helpers/ast_builder.h"
#include "test_helpers/lexer.h"
#include <gtest/gtest.h>

#include <memory>

TEST(CodeLexer, MainFunction_Trivial) {
  // fun main(): i32 { return 0; }
  std::queue<Token> input;
  input.emplace(keyword_function);
  input.emplace(name, "main");
  input.emplace(bracket_open);
  input.emplace(bracket_close);
  input.emplace(colon);
  input.emplace(name, "i32");
  input.emplace(curly_open);
  input.emplace(keyword_return);
  input.emplace(literal_integer, "0");
  input.emplace(semicolon);
  input.emplace(curly_close);

  auto expected = ASTBuilder::function("main", "i32") //
                      .returnLiteral(0);

  auto actual = CodeParser::parseTopLevelExpressions(input);

  ASSERT_EQ(actual.size(), 1);
  EXPECT_EQ(actual[0], expected);
}

TEST(CodeLexer, MainFunction_WithVariable) {
  // fun main(): i32 { let x = 32; return x; }
  std::queue<Token> input;
  input.emplace(keyword_function);
  input.emplace(name, "main");
  input.emplace(bracket_open);
  input.emplace(bracket_close);
  input.emplace(colon);
  input.emplace(name, "i32");
  input.emplace(curly_open);
  input.emplace(keyword_let);
  input.emplace(name, "x");
  input.emplace(operator_assign);
  input.emplace(literal_integer, "32");
  input.emplace(semicolon);
  input.emplace(keyword_return);
  input.emplace(name, "x");
  input.emplace(semicolon);
  input.emplace(curly_close);

  auto expected = ASTBuilder::function("main", "i32") //
                      .declareVariable("x", 32)       //
                      .returnVariable("x")
                      .build();

  auto actual = CodeParser::parseTopLevelExpressions(input);

  ASSERT_EQ(actual.size(), 1);
  EXPECT_EQ(actual[0], expected);
}