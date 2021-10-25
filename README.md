# BAFFL
Bill's Awesome For Funsies Language

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

### Default Types
* `bool` (with _true_ and _false_ literals)
* `i32` (with natural literals - 0, 1, 2, etc.)
* `string` (just passing around literals for now)
* _implicit_ `void` (on function return types)

## Things to do before 0.1
* Some kind of print function

## Things to do eventually
* Give our `if` an `else` branch
* Make our `if` and `while` more robust (it's finicky to leave blocks)
* Stop relying on clang/libc++ (for binary initialization)
* User-determined types
* Named & Optional arguments
* More numbers - unsigned, negative literals, and (maybe) floating-point numbers
* Proper error-handling instead of just `assert`ing and erroring out
