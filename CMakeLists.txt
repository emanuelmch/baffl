#
# Copyright (c) 2020 Emanuel Machado da Silva
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

cmake_minimum_required(VERSION 3.9) # CMP0069
cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0069 NEW)

project(BAFFL VERSION 0.1 LANGUAGES CXX)

if (NOT CMAKE_CXX_COMPILER_ID MATCHES "(Apple)?[Cc]lang")
    message(FATAL_ERROR "we need to be built by clang because of LLVM interop")
    #TODO: Do we, though?
endif ()

# Compiler flags
set(CMAKE_CXX_STANDARD 20)
#add_compile_options(-stdlib=libc++ -march=native -fno-rtti -fno-exceptions)
add_compile_options(-march=native -fno-rtti -fno-exceptions)
#add_compile_options(-Wno-unused-command-line-argument -Xlinker --export-dynamic)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -omit-frame-pointer")
# https://bugs.llvm.org//show_bug.cgi?id=32155 explains why -O0 over -Og
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Werror -Wall -Wextra -Wno-missing-braces -Wsign-conversion")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lLLVM-11")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lLLVM-11")

# Executable
add_executable(baffl
        src/cpp/ast.cpp
        src/cpp/code_emitter.cpp
        src/cpp/code_lexer.cpp
        src/cpp/code_parser.cpp
        src/cpp/main.cpp
        src/cpp/utils/file_reader.cpp
        )

# Testing
enable_testing()

set(integration_tests
        tests/integration/empty.baffl
        tests/integration/errorcode.baffl
        )

foreach (testfile ${integration_tests})
    get_filename_component(testname ${testfile} NAME_WE)
    set(testname integration/${testname})
    add_test(NAME ${testname}
            COMMAND ${CMAKE_SOURCE_DIR}/scripts/integration_test.sh $<TARGET_FILE:baffl> ${CMAKE_SOURCE_DIR}/${testfile})
endforeach (testfile)
