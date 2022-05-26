# BAFFL
Bill's Awesome For Funsies Language

## Interesting Design decisions
* Precedence of operators is always left to right
* Function arguments are always immutable (until I figure out how to do in/out arguments)

## Current state

### General features
* Functions (with return values/types) and arguments
* Variables
  * `let` for immutable 
  * `var` for mutable 
  * `=` for assignment 
* Basic mathematical functions (`+`, `-`)
* Basic comparison functions (`==`, `<` && `<=`)
* Statements end with `;` (for now)
* Conditions with `if`, loops with `while`
* Print things (with `import print;` for now)

### Default Types
* `bool` (with _true_ and _false_ literals)
* `i32` (with natural literals - 0, 1, 2, etc.)
* `string` (just passing around literals for now)
* _implicit_ `void` (on function return types)

## Things to do before 0.1
* Clean up the type usage (Move stuff into `TypeManager`)

## Things to do before 0.2
* Make function lookup mindful of the argument types, not just the function name
* Treat basic operations (e.g. i32 + i32) as regular (inline) functions internally

## Things to do before 1.0
* Self-host!

## Things to do eventually
* Give our `if` an `else` branch
* Make our `if` and `while` more robust (leaving blocks is a bit finicky)
* Stop relying on clang/libc++ (for binary initialization)
* Decide on how to do multi-module (i.e. something like `import` or `include`)
* User-determined types
* Uniform Function Call Syntax
* Named & Optional arguments
* More numbers - unsigned, negative, and floating-point numbers
* Proper error-handling instead of just `assert`ing and erroring out
