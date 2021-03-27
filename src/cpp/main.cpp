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

#include "code_emitter.h"
#include "code_lexer.h"
#include "code_parser.h"
#include "cpp/helpers/file_reader.h"

#include <iostream>

int main(int argc, char *argv[]) {
  if (argc != 4) {
    std::cerr << "Usage: baffl input.baffl -[v]o output" << std::endl;
    return 1;
  }

  bool isVerbose = false;
  {
    std::string outputParameterIndicator(argv[2]);
    if (outputParameterIndicator == "-vo") {
      isVerbose = true;
    } else if (outputParameterIndicator != "-o") {
      std::cerr << "Wrong arguments" << std::endl;
      return 1;
    }
  }

  std::string input(argv[1]);
  std::string output(argv[3]);

  auto fileContents = FileReader::readWholeFile(input);

  auto tokens = CodeLexer::tokenise(fileContents);
  if (tokens.empty()) {
    std::cerr << "Lexer couldn't create any tokens" << std::endl;
    return 1;
  }

  auto topLevel = CodeParser::parseTopLevelExpressions(tokens);

  return CodeEmitter::emitObjectFile(topLevel, output, isVerbose);
}
