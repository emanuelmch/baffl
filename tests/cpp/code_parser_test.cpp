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

#include <memory>

TEST(CodeLexer, MainFunction_Trivial) {
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

  auto expected = std::make_shared<FunctionAST>( //
      "main",                                    //
      "i32",                                     //
      ReturnAST{LiteralIntegerAST{0}}            //
  );

  auto actual = CodeParser::parseTopLevelExpressions(input);

  ASSERT_EQ(actual.size(), 1);
  EXPECT_EQ(actual[0], expected);
}

TEST(CodeLexer, Trivial_Incorrect) {
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

  auto expected = std::make_shared<FunctionAST>( //
      "main",                                    //
      "i32",                                     //
      ReturnAST{LiteralIntegerAST{0}}            //
  );

  auto actual = CodeParser::parseTopLevelExpressions(input);

  ASSERT_EQ(actual.size(), 1);
  EXPECT_NE(actual[0], expected);
}

// TEST(CodeLexer, MainFunction_WithVariable) {
//   std::queue<Token> input;
//   input.push(Token(keyword_function));
//   input.push(Token(name, "main"));
//   input.push(Token(bracket_open));
//   input.push(Token(bracket_close));
//   input.push(Token(colon));
//   input.push(Token(name, "i32"));
//   input.push(Token(curly_open));
//   input.push(Token(keyword_let));
//   input.push(Token(name, "x"));
//   input.push(Token(semicolon));
//   input.push(Token(name, "x"));
//   input.push(Token(operator_equal));
//   input.push(Token(literal_integer, "32"));
//   input.push(Token(semicolon));
//   input.push(Token(keyword_return));
//   input.push(Token(literal_integer, "0"));
//   input.push(Token(semicolon));
//   input.push(Token(curly_close));
//
//   auto expected = std::make_shared<FunctionAST>( //
//       "main",                                    //
//       "i32",                                     //
//           VariableDeclarationAST{"x"},                         //
//           AssignmentExpressionAST{"x", LiteralIntegerAST{32}}, //
//           ReturnAST{VariableReadAST{"x"}}                      //
//   );
//
//   auto actual = CodeParser::parseTopLevelExpressions(input);
//
//   ASSERT_EQ(actual.size(), 1);
//   EXPECT_NE(actual[0], expected);
// }