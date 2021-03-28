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

#include "test_helpers/ast.h"
#include "test_helpers/lexer.h"
#include <gtest/gtest.h>

TEST(CodeLexer, TrivialFunction) {
  auto input = "fun thisIsAFunction(): i32 {\n"
               "    return 0;\n"
               "}\n";

  std::queue<Token> expected;
  expected.emplace(keyword_function);
  expected.emplace(name, "thisIsAFunction");
  expected.emplace(bracket_open);
  expected.emplace(bracket_close);
  expected.emplace(colon);
  expected.emplace(name, "i32");
  expected.emplace(curly_open);
  expected.emplace(keyword_return);
  expected.emplace(literal_integer, "0");
  expected.emplace(semicolon);
  expected.emplace(curly_close);

  EXPECT_EQ(CodeLexer::tokenise(input), expected);
}

TEST(CodeLexer, FunctionWithVariable) {
  auto input = "fun main(): i32 {\n"
               "    let x = 32;\n"
               "    return x;\n"
               "}\n";

  std::queue<Token> expected;
  expected.emplace(keyword_function);
  expected.emplace(name, "main");
  expected.emplace(bracket_open);
  expected.emplace(bracket_close);
  expected.emplace(colon);
  expected.emplace(name, "i32");
  expected.emplace(curly_open);
  expected.emplace(keyword_let);
  expected.emplace(name, "x");
  expected.emplace(operator_assign);
  expected.emplace(literal_integer, "32");
  expected.emplace(semicolon);
  expected.emplace(keyword_return);
  expected.emplace(name, "x");
  expected.emplace(semicolon);
  expected.emplace(curly_close);

  EXPECT_EQ(CodeLexer::tokenise(input), expected);
}

TEST(CodeLexer, FunctionCall) {
  auto input = "fun main(): i32 {\n"
               "    return functionCall();\n"
               "}\n";

  std::queue<Token> expected;
  expected.emplace(keyword_function);
  expected.emplace(name, "main");
  expected.emplace(bracket_open);
  expected.emplace(bracket_close);
  expected.emplace(colon);
  expected.emplace(name, "i32");
  expected.emplace(curly_open);
  expected.emplace(keyword_return);
  expected.emplace(name, "functionCall");
  expected.emplace(bracket_open);
  expected.emplace(bracket_close);
  expected.emplace(semicolon);
  expected.emplace(curly_close);

  EXPECT_EQ(CodeLexer::tokenise(input), expected);
}

TEST(CodeLexer, MultipleFunction) {
  // fun f1(): i32 { return 1; } fun f2(): i32 { return 2; }
  auto input = "fun f1(): i32 { return 1; }\n"
               "fun f2(): i32 { return 2; }\n";

  std::queue<Token> expected;
  expected.emplace(keyword_function);
  expected.emplace(name, "f1");
  expected.emplace(bracket_open);
  expected.emplace(bracket_close);
  expected.emplace(colon);
  expected.emplace(name, "i32");
  expected.emplace(curly_open);
  expected.emplace(keyword_return);
  expected.emplace(literal_integer, "1");
  expected.emplace(semicolon);
  expected.emplace(curly_close);

  expected.emplace(keyword_function);
  expected.emplace(name, "f2");
  expected.emplace(bracket_open);
  expected.emplace(bracket_close);
  expected.emplace(colon);
  expected.emplace(name, "i32");
  expected.emplace(curly_open);
  expected.emplace(keyword_return);
  expected.emplace(literal_integer, "2");
  expected.emplace(semicolon);
  expected.emplace(curly_close);

  EXPECT_EQ(CodeLexer::tokenise(input), expected);
}