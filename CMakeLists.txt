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

cmake_minimum_required(VERSION 3.0)
project(BAFFL VERSION 0.1 LANGUAGES CXX)
include_directories(src/cpp)

if (NOT CMAKE_CXX_COMPILER_ID MATCHES "(Apple)?[Cc]lang")
    message(FATAL_ERROR "we need to be built by clang because of LLVM interop")
    #TODO: Do we, though?
endif ()

# Compiler flags
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-march=native -fno-exceptions)
#add_compile_options(-Wno-unused-command-line-argument -Xlinker --export-dynamic) I think we'll need this someday
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -omit-frame-pointer")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Werror -Wall -Wextra -Wno-missing-braces -Wsign-conversion")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/usr/lib64 -lLLVM")

# Source files
add_library(baffl_lib
        src/cpp/ast.cpp
        src/cpp/code_emitter.cpp
        src/cpp/code_lexer.cpp
        src/cpp/code_parser.cpp
        src/cpp/ast/emission_context.cpp
        )

add_executable(baffl
        src/cpp/main.cpp
        src/cpp/helpers/file_reader.cpp
        $<TARGET_OBJECTS:baffl_lib>
        )

# Testing
enable_testing()

# Unit tests
add_subdirectory(libs/googletest)
set(unit_tests
        tests/cpp/code_lexer_test.cpp
        tests/cpp/code_parser_test.cpp
        )

foreach (unit_test_file ${unit_tests})
    get_filename_component(unit_test_name ${unit_test_file} NAME_WE)
    add_executable(${unit_test_name} ${unit_test_file} $<TARGET_OBJECTS:baffl_lib>)
    target_link_libraries(${unit_test_name} gtest gtest_main pthread)

    add_test(${unit_test_name} ${unit_test_name})
endforeach (unit_test_file)


# Integration tests
set(integration_tests
        tests/integration/complex_function_plus.baffl
        tests/integration/empty.baffl
        tests/integration/error_code.baffl
        tests/integration/function_arguments.baffl
        tests/integration/function_call.baffl
        tests/integration/maths_plus.baffl
        tests/integration/variable.baffl
        tests/integration/variable_scope.baffl
        )

foreach (testfile ${integration_tests})
    get_filename_component(testname ${testfile} NAME_WE)
    set(testname integration/${testname})
    add_test(NAME ${testname}
            COMMAND ${CMAKE_SOURCE_DIR}/src/scripts/integration_test.sh $<TARGET_FILE:baffl> ${CMAKE_SOURCE_DIR}/${testfile})
endforeach (testfile)
