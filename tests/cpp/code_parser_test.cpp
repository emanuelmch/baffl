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

#include "test_helpers/test_ast.h"
#include "test_helpers/test_ast_builder.h"
#include "test_helpers/test_lexer.h"

#include <gtest/gtest.h>

#include <memory>

TEST(CodeParser, TrivialFunction) {
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
                      .returnIntLiteral(0);

  auto actual = CodeParser::parseTopLevelExpressions(input);

  ASSERT_EQ(actual.size(), 1);
  EXPECT_EQ(actual[0], expected);
}

TEST(CodeParser, FunctionWithVariable) {
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
                      .returnVariable("x");

  auto actual = CodeParser::parseTopLevelExpressions(input);

  ASSERT_EQ(actual.size(), 1);
  EXPECT_EQ(actual[0], expected);
}

TEST(CodeParser, FunctionCall) {
  // fun main(): i32 { return functionCall(); }
  std::queue<Token> input;
  input.emplace(keyword_function);
  input.emplace(name, "main");
  input.emplace(bracket_open);
  input.emplace(bracket_close);
  input.emplace(colon);
  input.emplace(name, "i32");
  input.emplace(curly_open);
  input.emplace(keyword_return);
  input.emplace(name, "functionCall");
  input.emplace(bracket_open);
  input.emplace(bracket_close);
  input.emplace(semicolon);
  input.emplace(curly_close);

  auto expected = ASTBuilder::function("main", "i32") //
                      .returnFunctionCall("functionCall");

  auto actual = CodeParser::parseTopLevelExpressions(input);

  ASSERT_EQ(actual.size(), 1);
  EXPECT_EQ(actual[0], expected);
}

TEST(CodeLexer, FunctionCallWithOneArgument) {
  // fun main(): i32 { return functionCall(123); }

  std::queue<Token> input;
  input.emplace(keyword_function);
  input.emplace(name, "main");
  input.emplace(bracket_open);
  input.emplace(bracket_close);
  input.emplace(colon);
  input.emplace(name, "i32");
  input.emplace(curly_open);
  input.emplace(keyword_return);
  input.emplace(name, "functionCall");
  input.emplace(bracket_open);
  input.emplace(literal_integer, "123");
  input.emplace(bracket_close);
  input.emplace(semicolon);
  input.emplace(curly_close);

  auto expected = ASTBuilder::function("main", "i32") //
                      .returnFunctionCall("functionCall", 123);

  auto actual = CodeParser::parseTopLevelExpressions(input);

  ASSERT_EQ(actual.size(), 1);
  EXPECT_EQ(actual[0], expected);
}

TEST(CodeParser, MultipleFunctions) {
  // fun f1(): i32 { return 1; } fun f2(): i32 { return 2; }

  std::queue<Token> input;
  input.emplace(keyword_function);
  input.emplace(name, "f1");
  input.emplace(bracket_open);
  input.emplace(bracket_close);
  input.emplace(colon);
  input.emplace(name, "i32");
  input.emplace(curly_open);
  input.emplace(keyword_return);
  input.emplace(literal_integer, "1");
  input.emplace(semicolon);
  input.emplace(curly_close);

  input.emplace(keyword_function);
  input.emplace(name, "f2");
  input.emplace(bracket_open);
  input.emplace(bracket_close);
  input.emplace(colon);
  input.emplace(name, "i32");
  input.emplace(curly_open);
  input.emplace(keyword_return);
  input.emplace(literal_integer, "2");
  input.emplace(semicolon);
  input.emplace(curly_close);

  auto f1 = ASTBuilder::function("f1", "i32").returnIntLiteral(1);
  auto f2 = ASTBuilder::function("f2", "i32").returnIntLiteral(2);

  auto actual = CodeParser::parseTopLevelExpressions(input);

  ASSERT_EQ(actual.size(), 2);
  EXPECT_EQ(actual[0], f1);
  EXPECT_EQ(actual[1], f2);
}

TEST(CodeParser, FunctionWithOneArgument) {
  // fun main(x: i32): i32 { return x; }

  std::queue<Token> input;
  input.emplace(keyword_function);
  input.emplace(name, "main");
  input.emplace(bracket_open);
  input.emplace(name, "x");
  input.emplace(colon);
  input.emplace(name, "i32");
  input.emplace(bracket_close);
  input.emplace(colon);
  input.emplace(name, "i32");
  input.emplace(curly_open);
  input.emplace(keyword_return);
  input.emplace(name, "x");
  input.emplace(semicolon);
  input.emplace(curly_close);

  auto expected = ASTBuilder::function("main", "i32") //
                      .addArgument("x", "i32")        //
                      .returnVariable("x");

  auto actual = CodeParser::parseTopLevelExpressions(input);

  ASSERT_EQ(actual.size(), 1);
  EXPECT_EQ(actual[0], expected);
}

TEST(CodeParser, BinaryOperator_Plus) {
  // fun main(): i32 { return 1 + 2; }

  std::queue<Token> input;
  input.emplace(keyword_function);
  input.emplace(name, "main");
  input.emplace(bracket_open);
  input.emplace(bracket_close);
  input.emplace(colon);
  input.emplace(name, "i32");
  input.emplace(curly_open);
  input.emplace(keyword_return);
  input.emplace(literal_integer, "1");
  input.emplace(operator_plus);
  input.emplace(literal_integer, "2");
  input.emplace(semicolon);
  input.emplace(curly_close);

  auto expected = ASTBuilder::function("main", "i32") //
                      .returnExpression([]() {        //
                        return literal(1)->plus(literal(2));
                      });

  auto actual = CodeParser::parseTopLevelExpressions(input);

  ASSERT_EQ(actual.size(), 1);
  EXPECT_EQ(actual[0], expected);
}

TEST(CodeParser, BinaryOperator_Minus) {
  // fun main(): i32 { return 1 - 2; }

  std::queue<Token> input;
  input.emplace(keyword_function);
  input.emplace(name, "main");
  input.emplace(bracket_open);
  input.emplace(bracket_close);
  input.emplace(colon);
  input.emplace(name, "i32");
  input.emplace(curly_open);
  input.emplace(keyword_return);
  input.emplace(literal_integer, "1");
  input.emplace(operator_minus);
  input.emplace(literal_integer, "2");
  input.emplace(semicolon);
  input.emplace(curly_close);

  auto expected = ASTBuilder::function("main", "i32") //
                      .returnExpression([]() {        //
                        return literal(1)->minus(literal(2));
                      });

  auto actual = CodeParser::parseTopLevelExpressions(input);

  ASSERT_EQ(actual.size(), 1);
  EXPECT_EQ(actual[0], expected);
}

TEST(CodeParser, BinaryOperator_Multiple) {
  // fun main(): i32 { return 1 + 2 + 3; }

  std::queue<Token> input;
  input.emplace(keyword_function);
  input.emplace(name, "main");
  input.emplace(bracket_open);
  input.emplace(bracket_close);
  input.emplace(colon);
  input.emplace(name, "i32");
  input.emplace(curly_open);
  input.emplace(keyword_return);
  input.emplace(literal_integer, "1");
  input.emplace(operator_plus);
  input.emplace(literal_integer, "2");
  input.emplace(operator_plus);
  input.emplace(literal_integer, "3");
  input.emplace(semicolon);
  input.emplace(curly_close);

  auto expected = ASTBuilder::function("main", "i32") //
                      .returnExpression([=]() {       //
                        return literal(1)
                            ->plus(literal(2))  //
                            ->plus(literal(3)); //
                      });

  auto actual = CodeParser::parseTopLevelExpressions(input);

  ASSERT_EQ(actual.size(), 1);
  EXPECT_EQ(actual[0], expected);
}

TEST(CodeParser, BoolFunction_True) {
  // fun returnsTrue(): bool { return true; }
  std::queue<Token> input;
  input.emplace(keyword_function);
  input.emplace(name, "returnsTrue");
  input.emplace(bracket_open);
  input.emplace(bracket_close);
  input.emplace(colon);
  input.emplace(name, "bool");
  input.emplace(curly_open);
  input.emplace(keyword_return);
  input.emplace(keyword_true);
  input.emplace(semicolon);
  input.emplace(curly_close);

  auto expected = ASTBuilder::function("returnsTrue", "bool") //
                      .returnBoolLiteral(true);

  auto actual = CodeParser::parseTopLevelExpressions(input);

  ASSERT_EQ(actual.size(), 1);
  EXPECT_EQ(actual[0], expected);
}

TEST(CodeParser, BoolFunction_False) {
  // fun returnsFalse(): bool { return false; }
  std::queue<Token> input;
  input.emplace(keyword_function);
  input.emplace(name, "returnsFalse");
  input.emplace(bracket_open);
  input.emplace(bracket_close);
  input.emplace(colon);
  input.emplace(name, "bool");
  input.emplace(curly_open);
  input.emplace(keyword_return);
  input.emplace(keyword_false);
  input.emplace(semicolon);
  input.emplace(curly_close);

  auto expected = ASTBuilder::function("returnsFalse", "bool") //
                      .returnBoolLiteral(false);

  auto actual = CodeParser::parseTopLevelExpressions(input);

  ASSERT_EQ(actual.size(), 1);
  EXPECT_EQ(actual[0], expected);
}

TEST(CodeParser, BoolParameter) {
  // fun randomFunction(x: bool) {}

  std::queue<Token> input;
  input.emplace(keyword_function);
  input.emplace(name, "randomFunction");
  input.emplace(bracket_open);
  input.emplace(name, "x");
  input.emplace(colon);
  input.emplace(name, "bool");
  input.emplace(bracket_close);
  input.emplace(curly_open);
  input.emplace(curly_close);

  auto expected = ASTBuilder::function("randomFunction", "void") //
                      .addArgument("x", "bool");

  auto actual = CodeParser::parseTopLevelExpressions(input);

  ASSERT_EQ(actual.size(), 1);
  EXPECT_EQ(actual[0], expected);
}
