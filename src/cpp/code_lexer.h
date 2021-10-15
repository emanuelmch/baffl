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

#pragma once

#include <cstdint>
#include <ostream>
#include <queue>
#include <string>
#include <utility>

enum TokenType : uint8_t {
  noop = 0,                       // no-op
  bracket_open,                   // (
  bracket_close,                  // )
  curly_open,                     // {
  curly_close,                    // }
  colon,                          // :
  semicolon,                      // ;
  comma,                          // ,
  operator_assign,                // =
  operator_plus,                  // +
  operator_minus,                 // -
  operator_equals,                // ==
  operator_less_than,             // <
  operator_less_than_or_equal_to, // <=
  name,                           // name
  literal_integer,                // integer
  keyword_true,                   // true
  keyword_false,                  // false
  keyword_function,               // fun
  keyword_let,                    // let
  keyword_var,                    // var
  keyword_return,                 // return
  keyword_if,                     // if
  keyword_while,                  // while
};

struct Token {
  explicit Token(TokenType id) : _id(id) {}
  Token(TokenType id, uintmax_t v) : _id(id), _value(std::to_string(v)) {}
  Token(TokenType id, std::string v) : _id(id), _value(std::move(v)) {}
  ~Token() = default;

  [[nodiscard]] inline auto id() const { return _id; }
  [[nodiscard]] inline uintmax_t valueAsInt() const { return static_cast<uintmax_t>(std::stoll(_value)); }
  [[nodiscard]] inline std::string valueAsString() const { return _value; }
  [[nodiscard]] inline bool operator==(const Token &o) const { return _id == o._id && _value == o._value; }

private:
  TokenType _id;
  std::string _value;
};

std::ostream &operator<<(std::ostream &os, const Token &token);

namespace CodeLexer {

std::queue<Token> tokenise(const std::string &);
}
