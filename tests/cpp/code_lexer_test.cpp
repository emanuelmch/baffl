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

#include "cpp/code_lexer.h"

#include "helpers/ast.h"
#include "helpers/lexer.h"
#include <gtest/gtest.h>

TEST(CodeLexer, MainFunction_Trivial) {
  auto input = "fun main(): i32 {\n"
               "    return 0;\n"
               "}\n";

  std::queue<Token> expected;
  expected.push(Token(keyword_function));
  expected.push(Token(name, "main"));
  expected.push(Token(bracket_open));
  expected.push(Token(bracket_close));
  expected.push(Token(colon));
  expected.push(Token(name, "i32"));
  expected.push(Token(curly_open));
  expected.push(Token(keyword_return));
  expected.push(Token(literal_integer, "0"));
  expected.push(Token(semicolon));
  expected.push(Token(curly_close));

  EXPECT_EQ(CodeLexer::tokenise(input), expected);
}

TEST(CodeLexer, MainFunction_Trivial_Incorrect) {
  auto input = "fun main(): i32 {\n"
               "    return 0;\n"
               "}\n";

  std::queue<Token> expected;
  expected.push(Token(keyword_function));
  expected.push(Token(name, "main"));
  expected.push(Token(bracket_open));
  expected.push(Token(bracket_close));
  expected.push(Token(colon));
  expected.push(Token(name, "i32"));
  expected.push(Token(curly_open));
  expected.push(Token(keyword_return));
  expected.push(Token(literal_integer, "1"));
  expected.push(Token(semicolon));
  expected.push(Token(curly_close));

  EXPECT_NE(CodeLexer::tokenise(input), expected);
}

TEST(CodeLexer, MainFunction_WithVariable) {
  auto input = "fun main(): i32 {\n"
               "    let x = 32;\n"
               "    return 0;\n"
               "}\n";

  std::queue<Token> expected;
  expected.push(Token(keyword_function));
  expected.push(Token(name, "main"));
  expected.push(Token(bracket_open));
  expected.push(Token(bracket_close));
  expected.push(Token(colon));
  expected.push(Token(name, "i32"));
  expected.push(Token(curly_open));
  expected.push(Token(keyword_let));
  expected.push(Token(name, "x"));
  expected.push(Token(operator_equal));
  expected.push(Token(literal_integer, "32"));
  expected.push(Token(semicolon));
  expected.push(Token(keyword_return));
  expected.push(Token(literal_integer, "0"));
  expected.push(Token(semicolon));
  expected.push(Token(curly_close));

  EXPECT_EQ(CodeLexer::tokenise(input), expected);
}
