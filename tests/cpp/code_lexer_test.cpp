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

#include "code_lexer.h"

#include "test_helpers/test_ast.h"
#include "test_helpers/test_lexer.h"

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

TEST(CodeLexer, FunctionCallWithOneArgument) {
  auto input = "fun main(): i32 {\n"
               "    return functionCall(123);\n"
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
  expected.emplace(literal_integer, "123");
  expected.emplace(bracket_close);
  expected.emplace(semicolon);
  expected.emplace(curly_close);

  EXPECT_EQ(CodeLexer::tokenise(input), expected);
}

TEST(CodeLexer, FunctionCallWithMultipleArguments) {
  auto input = "fun main(): i32 { return functionCall(1, 2, 3); }";

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
  expected.emplace(literal_integer, "1");
  expected.emplace(comma);
  expected.emplace(literal_integer, "2");
  expected.emplace(comma);
  expected.emplace(literal_integer, "3");
  expected.emplace(bracket_close);
  expected.emplace(semicolon);
  expected.emplace(curly_close);

  EXPECT_EQ(CodeLexer::tokenise(input), expected);
}

TEST(CodeLexer, MultipleFunctions) {
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

TEST(CodeLexer, FunctionWithOneArgument) {
  auto input = "fun main(x: i32): i32 { return x; }";

  std::queue<Token> expected;
  expected.emplace(keyword_function);
  expected.emplace(name, "main");
  expected.emplace(bracket_open);
  expected.emplace(name, "x");
  expected.emplace(colon);
  expected.emplace(name, "i32");
  expected.emplace(bracket_close);
  expected.emplace(colon);
  expected.emplace(name, "i32");
  expected.emplace(curly_open);
  expected.emplace(keyword_return);
  expected.emplace(name, "x");
  expected.emplace(semicolon);
  expected.emplace(curly_close);

  EXPECT_EQ(CodeLexer::tokenise(input), expected);
}

TEST(CodeLexer, FunctionWithMultipleArguments) {
  auto input = "fun main(x: i32, y: i32, z: i32): i32 { return x + y + z; }";

  std::queue<Token> expected;
  expected.emplace(keyword_function);
  expected.emplace(name, "main");
  expected.emplace(bracket_open);
  expected.emplace(name, "x");
  expected.emplace(colon);
  expected.emplace(name, "i32");
  expected.emplace(comma);
  expected.emplace(name, "y");
  expected.emplace(colon);
  expected.emplace(name, "i32");
  expected.emplace(comma);
  expected.emplace(name, "z");
  expected.emplace(colon);
  expected.emplace(name, "i32");
  expected.emplace(bracket_close);
  expected.emplace(colon);
  expected.emplace(name, "i32");
  expected.emplace(curly_open);
  expected.emplace(keyword_return);
  expected.emplace(name, "x");
  expected.emplace(operator_plus);
  expected.emplace(name, "y");
  expected.emplace(operator_plus);
  expected.emplace(name, "z");
  expected.emplace(semicolon);
  expected.emplace(curly_close);

  EXPECT_EQ(CodeLexer::tokenise(input), expected);
}

TEST(CodeLexer, MutableVariables) {
  auto input = "fun xy() { var x = 1; x = 2; }";

  std::queue<Token> expected;
  expected.emplace(keyword_function);
  expected.emplace(name, "xy");
  expected.emplace(bracket_open);
  expected.emplace(bracket_close);
  expected.emplace(curly_open);
  expected.emplace(keyword_var);
  expected.emplace(name, "x");
  expected.emplace(operator_assign);
  expected.emplace(literal_integer, "1");
  expected.emplace(semicolon);
  expected.emplace(name, "x");
  expected.emplace(operator_assign);
  expected.emplace(literal_integer, "2");
  expected.emplace(semicolon);
  expected.emplace(curly_close);

  EXPECT_EQ(CodeLexer::tokenise(input), expected);
}

TEST(CodeLexer, BinaryOperator_Plus) {
  auto input = "fun main(): i32 { return 1 + 2; }";

  std::queue<Token> expected;
  expected.emplace(keyword_function);
  expected.emplace(name, "main");
  expected.emplace(bracket_open);
  expected.emplace(bracket_close);
  expected.emplace(colon);
  expected.emplace(name, "i32");
  expected.emplace(curly_open);
  expected.emplace(keyword_return);
  expected.emplace(literal_integer, "1");
  expected.emplace(operator_plus);
  expected.emplace(literal_integer, "2");
  expected.emplace(semicolon);
  expected.emplace(curly_close);

  EXPECT_EQ(CodeLexer::tokenise(input), expected);
}

TEST(CodeLexer, BinaryOperator_Minus) {
  auto input = "fun main(): i32 { return 1 - 2; }";

  std::queue<Token> expected;
  expected.emplace(keyword_function);
  expected.emplace(name, "main");
  expected.emplace(bracket_open);
  expected.emplace(bracket_close);
  expected.emplace(colon);
  expected.emplace(name, "i32");
  expected.emplace(curly_open);
  expected.emplace(keyword_return);
  expected.emplace(literal_integer, "1");
  expected.emplace(operator_minus);
  expected.emplace(literal_integer, "2");
  expected.emplace(semicolon);
  expected.emplace(curly_close);

  EXPECT_EQ(CodeLexer::tokenise(input), expected);
}

TEST(CodeLexer, BinaryOperator_Division) {
  auto input = "fun main(): i32 { return 9 / 3; }";

  std::queue<Token> expected;
  expected.emplace(keyword_function);
  expected.emplace(name, "main");
  expected.emplace(bracket_open);
  expected.emplace(bracket_close);
  expected.emplace(colon);
  expected.emplace(name, "i32");
  expected.emplace(curly_open);
  expected.emplace(keyword_return);
  expected.emplace(literal_integer, "9");
  expected.emplace(operator_division);
  expected.emplace(literal_integer, "3");
  expected.emplace(semicolon);
  expected.emplace(curly_close);

  EXPECT_EQ(CodeLexer::tokenise(input), expected);
}

TEST(CodeLexer, BinaryOperator_Modulo) {
  auto input = "fun main(): i32 { return 1 % 2; }";

  std::queue<Token> expected;
  expected.emplace(keyword_function);
  expected.emplace(name, "main");
  expected.emplace(bracket_open);
  expected.emplace(bracket_close);
  expected.emplace(colon);
  expected.emplace(name, "i32");
  expected.emplace(curly_open);
  expected.emplace(keyword_return);
  expected.emplace(literal_integer, "1");
  expected.emplace(operator_modulo);
  expected.emplace(literal_integer, "2");
  expected.emplace(semicolon);
  expected.emplace(curly_close);

  EXPECT_EQ(CodeLexer::tokenise(input), expected);
}

TEST(CodeLexer, BinaryOperator_Multiple) {
  auto input = "fun main(): i32 { return 1 + 2 + 3; }";

  std::queue<Token> expected;
  expected.emplace(keyword_function);
  expected.emplace(name, "main");
  expected.emplace(bracket_open);
  expected.emplace(bracket_close);
  expected.emplace(colon);
  expected.emplace(name, "i32");
  expected.emplace(curly_open);
  expected.emplace(keyword_return);
  expected.emplace(literal_integer, "1");
  expected.emplace(operator_plus);
  expected.emplace(literal_integer, "2");
  expected.emplace(operator_plus);
  expected.emplace(literal_integer, "3");
  expected.emplace(semicolon);
  expected.emplace(curly_close);

  EXPECT_EQ(CodeLexer::tokenise(input), expected);
}

TEST(CodeLexer, BoolFunction_True) {
  auto input = "fun returnsTrue(): bool { return true; }";

  std::queue<Token> expected;
  expected.emplace(keyword_function);
  expected.emplace(name, "returnsTrue");
  expected.emplace(bracket_open);
  expected.emplace(bracket_close);
  expected.emplace(colon);
  expected.emplace(name, "bool");
  expected.emplace(curly_open);
  expected.emplace(keyword_return);
  expected.emplace(keyword_true);
  expected.emplace(semicolon);
  expected.emplace(curly_close);

  EXPECT_EQ(CodeLexer::tokenise(input), expected);
}

TEST(CodeLexer, BoolFunction_False) {
  auto input = "fun returnsFalse(): bool { return false; }";

  std::queue<Token> expected;
  expected.emplace(keyword_function);
  expected.emplace(name, "returnsFalse");
  expected.emplace(bracket_open);
  expected.emplace(bracket_close);
  expected.emplace(colon);
  expected.emplace(name, "bool");
  expected.emplace(curly_open);
  expected.emplace(keyword_return);
  expected.emplace(keyword_false);
  expected.emplace(semicolon);
  expected.emplace(curly_close);

  EXPECT_EQ(CodeLexer::tokenise(input), expected);
}

TEST(CodeLexer, BoolParameter) {
  auto input = "fun randomFunction(x: bool) {}";

  std::queue<Token> expected;
  expected.emplace(keyword_function);
  expected.emplace(name, "randomFunction");
  expected.emplace(bracket_open);
  expected.emplace(name, "x");
  expected.emplace(colon);
  expected.emplace(name, "bool");
  expected.emplace(bracket_close);
  expected.emplace(curly_open);
  expected.emplace(curly_close);

  EXPECT_EQ(CodeLexer::tokenise(input), expected);
}

TEST(CodeLexer, StringLiterals) {
  auto input = "fun function() { let x = \"Hi\"; }";

  std::queue<Token> expected;
  expected.emplace(keyword_function);
  expected.emplace(name, "function");
  expected.emplace(bracket_open);
  expected.emplace(bracket_close);
  expected.emplace(curly_open);
  expected.emplace(keyword_let);
  expected.emplace(name, "x");
  expected.emplace(operator_assign);
  expected.emplace(literal_string, "Hi");
  expected.emplace(semicolon);
  expected.emplace(curly_close);

  EXPECT_EQ(CodeLexer::tokenise(input), expected);
}

TEST(CodeLexer, OperatorEquals) {
  auto input = "fun function(x: bool): bool { return x == true; }";

  std::queue<Token> expected;
  expected.emplace(keyword_function);
  expected.emplace(name, "function");
  expected.emplace(bracket_open);
  expected.emplace(name, "x");
  expected.emplace(colon);
  expected.emplace(name, "bool");
  expected.emplace(bracket_close);
  expected.emplace(colon);
  expected.emplace(name, "bool");
  expected.emplace(curly_open);
  expected.emplace(keyword_return);
  expected.emplace(name, "x");
  expected.emplace(operator_equals);
  expected.emplace(keyword_true);
  expected.emplace(semicolon);
  expected.emplace(curly_close);

  EXPECT_EQ(CodeLexer::tokenise(input), expected);
}

TEST(CodeLexer, IfWithBoolean) {
  auto input = "fun function(x: bool): i32 { if (x) { return 1; } return 0; }";

  std::queue<Token> expected;
  expected.emplace(keyword_function);
  expected.emplace(name, "function");
  expected.emplace(bracket_open);
  expected.emplace(name, "x");
  expected.emplace(colon);
  expected.emplace(name, "bool");
  expected.emplace(bracket_close);
  expected.emplace(colon);
  expected.emplace(name, "i32");
  expected.emplace(curly_open);
  expected.emplace(keyword_if);
  expected.emplace(bracket_open);
  expected.emplace(name, "x");
  expected.emplace(bracket_close);
  expected.emplace(curly_open);
  expected.emplace(keyword_return);
  expected.emplace(literal_integer, 1);
  expected.emplace(semicolon);
  expected.emplace(curly_close);
  expected.emplace(keyword_return);
  expected.emplace(literal_integer, 0);
  expected.emplace(semicolon);
  expected.emplace(curly_close);

  EXPECT_EQ(CodeLexer::tokenise(input), expected);
}

TEST(CodeLexer, IfWithExpression) {
  auto input = "fun function(): i32 { if (1 == 1) { return 1; } return 0; }";

  std::queue<Token> expected;
  expected.emplace(keyword_function);
  expected.emplace(name, "function");
  expected.emplace(bracket_open);
  expected.emplace(bracket_close);
  expected.emplace(colon);
  expected.emplace(name, "i32");
  expected.emplace(curly_open);
  expected.emplace(keyword_if);
  expected.emplace(bracket_open);
  expected.emplace(literal_integer, 1);
  expected.emplace(operator_equals);
  expected.emplace(literal_integer, 1);
  expected.emplace(bracket_close);
  expected.emplace(curly_open);
  expected.emplace(keyword_return);
  expected.emplace(literal_integer, 1);
  expected.emplace(semicolon);
  expected.emplace(curly_close);
  expected.emplace(keyword_return);
  expected.emplace(literal_integer, 0);
  expected.emplace(semicolon);
  expected.emplace(curly_close);

  EXPECT_EQ(CodeLexer::tokenise(input), expected);
}

TEST(CodeLexer, While) {
  auto input = "fun x(): i32 { while (true) { return 1; } }";

  std::queue<Token> expected;
  expected.emplace(keyword_function);
  expected.emplace(name, "x");
  expected.emplace(bracket_open);
  expected.emplace(bracket_close);
  expected.emplace(colon);
  expected.emplace(name, "i32");
  expected.emplace(curly_open);
  expected.emplace(keyword_while);
  expected.emplace(bracket_open);
  expected.emplace(keyword_true);
  expected.emplace(bracket_close);
  expected.emplace(curly_open);
  expected.emplace(keyword_return);
  expected.emplace(literal_integer, 1);
  expected.emplace(semicolon);
  expected.emplace(curly_close);
  expected.emplace(curly_close);

  EXPECT_EQ(CodeLexer::tokenise(input), expected);
}

TEST(CodeLexer, LessThan) {
  auto input = "fun lessThanTwo(x: i32): bool { return x < 2; }";

  std::queue<Token> expected;
  expected.emplace(keyword_function);
  expected.emplace(name, "lessThanTwo");
  expected.emplace(bracket_open);
  expected.emplace(name, "x");
  expected.emplace(colon);
  expected.emplace(name, "i32");
  expected.emplace(bracket_close);
  expected.emplace(colon);
  expected.emplace(name, "bool");
  expected.emplace(curly_open);
  expected.emplace(keyword_return);
  expected.emplace(name, "x");
  expected.emplace(operator_less_than);
  expected.emplace(literal_integer, 2);
  expected.emplace(semicolon);
  expected.emplace(curly_close);

  EXPECT_EQ(CodeLexer::tokenise(input), expected);
}

TEST(CodeLexer, LessThanOrEqualTo) {
  auto input = "fun twoOrLess(x: i32): bool { return x <= 2; }";

  std::queue<Token> expected;
  expected.emplace(keyword_function);
  expected.emplace(name, "twoOrLess");
  expected.emplace(bracket_open);
  expected.emplace(name, "x");
  expected.emplace(colon);
  expected.emplace(name, "i32");
  expected.emplace(bracket_close);
  expected.emplace(colon);
  expected.emplace(name, "bool");
  expected.emplace(curly_open);
  expected.emplace(keyword_return);
  expected.emplace(name, "x");
  expected.emplace(operator_less_than_or_equal_to);
  expected.emplace(literal_integer, 2);
  expected.emplace(semicolon);
  expected.emplace(curly_close);

  EXPECT_EQ(CodeLexer::tokenise(input), expected);
}

TEST (CodeLexer, Import) {
  auto input = "import print; fun nothing() {}";

  std::queue<Token> expected;
  expected.emplace(keyword_import);
  expected.emplace(name, "print");
  expected.emplace(semicolon);
  expected.emplace(keyword_function);
  expected.emplace(name, "nothing");
  expected.emplace(bracket_open);
  expected.emplace(bracket_close);
  expected.emplace(curly_open);
  expected.emplace(curly_close);

  EXPECT_EQ(CodeLexer::tokenise(input), expected);
}
