# Vctr

A simple stack based programming language made in C. 

## Stack notation
(before -- after)

## Operations

### Arithmatic

| Op | Effect | Description | Status | 
| - | - | - | - | 
| + | a b -- a | Adds the two elemnts on top of the stack | [Done] |
| - | a b -- a | Substracts the two elemnts on top of the stack | [Done] |
| * | a b -- a | Multiplies the two elemnts on top of the stack | [Done] |
| / | a b -- a | Divides the two elemnts on top of the stack | [Done] |

### Comparison

| Op | Effect | Description | Status | 
| - | - | - | - | 
| == | a b -- a | Checks if two elements on top of the stack equal | [DONE] |
| != | a b -- a | Checks if two elements on top of the stack not equal | [DONE] |
| >  | a b -- a | Checks if the first element on top of the stack greater than second element on top of the stack | [DONE] |
| >= | a b -- a | Checks if the first element on top of the stack greater or equal than second element on top of the stack | [DONE] |
| <  | a b -- a | Checks if the first element on top of the stack less than second element on top of the stack | [DONE] |
| <= | a b -- a | Checks if the first element on top of the stack less or equal than second element on top of the stack | [DONE] |

### Stack manipulation
| Op | Effect | Description | Status | 
| - | - | - | - | 
| dup  | a -- a | Duplicates the element on top of the stack | [DONE] |
| over | a b -- a b a | Duplicates the 2nd element on the stack to the top | [DONE]|
| swap | a b -- b a | Swaps the top two elements on the stack| [DONE] | 
| rot  | a b c -- b c a | Pushes the 3rd element on the stack to the top | [DONE]
| drop | a b -- a | Drops the elemnt on top of the stack | [DONE] |

### I/O 
 
| Op | Effect | Description |
| - | - | - |
| Print | a -- | Prints the top element of the stack | [DONE]
| Exit  | a -- | Exits the program with the give exit value| [wip]

### Control
TODO: Impalemnt else
#### If
````
<condition> if <body> else* <body> end 
````
#### While
````
while <condition> do <body> end 
````
