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

#include <cassert>
#include <iostream>

std::ostream &operator<<(std::ostream &os, const Token &token) {
  switch (token.id()) {
  case noop:
    os << "noop";
    break;
  case bracket_open:
    os << "`(`";
    break;
  case bracket_close:
    os << "`)`";
    break;
  case curly_open:
    os << "`{`";
    break;
  case curly_close:
    os << "`}`";
    break;
  case colon:
    os << "`:`";
    break;
  case semicolon:
    os << "`;`";
    break;
  case operator_assign:
    os << "`=`";
    break;
  case name:
    os << "name: [" << token.valueAsString() << "]";
    break;
  case literal_integer:
    os << "literal: int: [" << token.valueAsString() << "]";
    break;
  case keyword_function:
    os << "keyword: function";
    break;
  case keyword_let:
    os << "keyword: let";
    break;
  case keyword_return:
    os << "keyword: return";
    break;
  }
  return os;
}

inline Token fromString(const std::string_view &token) {
  assert(!token.empty());

  if (token == "fun") {
    return Token(keyword_function);
  } else if (token == "let") {
    return Token(keyword_let);
  } else if (token == "return") {
    return Token(keyword_return);
  }

  return Token(name, std::string(token));
}

inline std::string_view readTokenWhile(std::basic_string_view<char> view, auto predicate) {
  assert(predicate(view.front()));
  for (size_t i = 1; i < view.size(); ++i) {
    auto nextChar = view.at(i);
    if (predicate(nextChar) == false) {
      return view.substr(0, i);
    }
  }

  return view;
}

inline Token getNext(const std::string_view &content, size_t *pos) {
  while (std::isspace(content[*pos]))
    ++(*pos);

  if (*pos == content.size()) return Token(noop);

  auto currentChar = content[*pos];
  if (std::isalpha(currentChar)) {
    auto token = readTokenWhile(content.substr(*pos), [](auto it) { return std::isalnum(it); });
    (*pos) += token.size();
    return fromString(token);
  }

  if (std::isdigit(currentChar)) {
    // TODO: read floating point. Maybe?
    auto token = readTokenWhile(content.substr(*pos), [](auto it) { return std::isdigit(it); });
    (*pos) += token.size();
    return Token(literal_integer, std::string(token));
  }

  switch (currentChar) {
  case '(':
    ++(*pos);
    return Token(bracket_open);
  case ')':
    ++(*pos);
    return Token(bracket_close);
  case '{':
    ++(*pos);
    return Token(curly_open);
  case '}':
    ++(*pos);
    return Token(curly_close);
  case ':':
    ++(*pos);
    return Token(colon);
  case ';':
    ++(*pos);
    return Token(semicolon);
  case '=':
    ++(*pos);
    return Token(operator_assign);
  default:
    auto line = content.substr(*pos);
    auto linebreak = line.find_first_of("\r\n");
    std::cerr << "Couldn't read next token: " << line.substr(0, linebreak) << std::endl;
    return Token(noop);
  }
}

std::queue<Token> CodeLexer::tokenise(const std::string &content) {
  std::queue<Token> tokens;
  size_t pos = 0;

  auto token = getNext(content, &pos);
  while (token.id() != noop) {
    tokens.push(token);
    token = getNext(content, &pos);
  }

  return tokens;
}
