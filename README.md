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
* Basic comparison functions (`==` & `<`)
* Statements end with `;` (for now)

### Default Types
* `bool` (alongide _true_ and _false_ literals)
* `i32` (alongside natural literals - 0, 1, 2, etc.)
* _implicit_ `void` (on function return types)

## Things to do before 0.1
* A better `if` (and maybe even an `else`)
* One kind of loop (I'm thinking `while`)
* Some kind of print function

## Things to do eventually
* Stop relying on clang/libc++ (for binary initialization)
* User-determined types
* Named & Optional arguments
