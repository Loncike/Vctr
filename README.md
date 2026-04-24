# Vctr

A simple stack based programming language made in C. 

## TODO
| Status | Priority | Task | Description |
| - | - | - | - |
| Planned | 90 | Error handling | Add error handling for lexer, and runtime eval |  
| Planned | 70 | String literals | Add support for declaring c-style strings like: ````"Hello, world" 0 store```` |  
| Planned | 40 | Macros | | 
| Planned | 20 | Structs |  | 
| Planned | 10 | Functions | Maybe inspired by wasm | 
| Planned | 10 | Type checking | like how wasm does it |

### Completed
| Status | Priority | Task | Description |
| - | - | - | - |
| Done | 50 | Comments | Add support for comments | 

## Stack notation
(before -- after)

## Operations

### Arithmatic

| Op | Effect | Description | Status | 
| - | - | - | - | 
| + | a b -- a(b + a) | Adds the two elemnts on top of the stack | [Done] |
| - | a b -- a(b - a) | Substracts the two elemnts on top of the stack | [Done] |
| * | a b -- a(b * a) | Multiplies the two elemnts on top of the stack | [Done] |
| / | a b -- a(b / a) | Divides the two elemnts on top of the stack | [Done] |
| % | a b -- a(b % a) | Performs modulo operation on the two elements on top of the stack | [DONE]

### Comparison

| Op | Effect | Description | Status | 
| - | - | - | - | 
| == | a b -- a(b == a) | Checks if two elements on top of the stack equal | [DONE] |
| != | a b -- a(b != a) | Checks if two elements on top of the stack not equal | [DONE] |
| >  | a b -- a(b > a)  | Checks if the first element on top of the stack greater than second element on top of the stack | [DONE] |
| >= | a b -- a(b >= a) | Checks if the first element on top of the stack greater or equal than second element on top of the stack | [DONE] |
| <  | a b -- a(b < a)  | Checks if the first element on top of the stack less than second element on top of the stack | [DONE] |
| <= | a b -- a(b <= a) | Checks if the first element on top of the stack less or equal than second element on top of the stack | [DONE] |

### Stack manipulation
| Op | Effect | Description | Status | 
| - | - | - | - | 
| dup  | a -- a | Duplicates the element on top of the stack | [DONE] |
| over | a b -- a b a | Duplicates the 2nd element on the stack to the top | [DONE]|
| swap | a b -- b a | Swaps the top two elements on the stack| [DONE] | 
| rot  | a b c -- b c a | Pushes the 3rd element on the stack to the top | [DONE]
| drop | a b -- a | Drops the elemnt on top of the stack | [DONE] |

### I/O 
 
| Op | Effect | Description | Status |
| - | - | - |- |
| dump | a -- | Prints the top element of the stack | [DONE]
| exit  | a -- | Exits the program with the give exit value| [DONE]

### Memory
| Op | Effect | Description | Status |
| - | - | - |- |
| store | a: i8 -- | Stores the element on top of the stack at the given point on the heap in 1 byte| [DONE]
| load  | -- a: i8 | Loads value at the given point on the heap to the top of the stack in 1 byte| [DONE]
| store8 | a: i64 --| Stores the top element of the stack at the given point on the heap in 8 bytes | [DONE] 
| load8  | -- a: i64 | Loads value at the given point on the heap to the top of the stack in 8 byte| [DONE]
| ptr | -- a: *void | Converts heap pointer into c pointer | [DONE]

### System
#### syscall
````
<NR> <arg0..5>* <argN> syscall 
````

### Control flow
#### if
````
<condition> if <body> else* <body> end 
````
#### while
````
while <condition> do <body> end 
````
