# Kuan

| ![Kuan logo](media/Kuan-7.png) | Tiny dynamic s-expression programming language, with delusions of grandeur. |
| - | - |

**Inspirations**: [Chika](https://github.com/phunanon/Chika), [Ephem](https://github.com/phunanon/Ephem), [Wren](https://github.com/wren-lang/wren)

### Goals & Features

(NYA) - Not Yet Achieved

- S-expressions
- Speed
- [ARC](https://en.wikipedia.org/wiki/Automatic_Reference_Counting) memory management
- REPL
- Lazy enumerables (NYA)
  - List
  - Map
  - Range, Cycle, Emit, Repeat
  - Set
  - Network streams
  - File streams
- Anonymous functions
- Easy imports (NYA)

**Anti-features / trade-offs / corner-cutting**  
- No program errors
- Leaked memory with inappropriate use
- No closures

### Approaches

- Bytecode
  - Different native operations for fixed-arg and varadic
- Computed GOTO's
- Double-only arithmetic
  - NaN tagging
- ARC mem management only where objs are args
  - ARC mem leak check upon termination
- Valgrind memleak checks

### Usage

**Ubuntu/Debian**  
`git clone --depth=1 https://github.com/phunanon/Kuan`  
Ensure CMake in installed on your system.  
Run `./init.sh`. If needing to subsequently recompile use `./make.sh`.  
Execute `./build/kuan` in the terminal for the REPL,  
with a file path as the command-line argument to parse and execute,  
with `-r` as the second command-line argument to return last value.

### Syntax and native operations

### Literals and values

| Representations                      | Data type               |
| ------------------------------------ | ----------------------- |
| `T F N`                              | true, false, nil        |
| `\a`                                 | character               |
| `0x1` or `0x12`                      | 8 bit unsigned integer  |
| `3.14` or `.14` or `-3.14` or `-.14` | 64 bit signed real      |
| `0x3.AB` or `-3.AB` (hex)            | 64 bit signed real      |
| `[1 2 3]`                            | vector                  |
| `%`                                  | first parameter         |
| `%N`                                 | `N`th parameter         |
| `args`                               | vector of arguments     |

- Note: referencing more parameters than arguments given is undefined behaviour

### Functions and native operations

**Glossary**  
`[…]` representation of a vector.  
`[0..]` zero or more arguments.  
`[1..]` one or more arguments.  
`[arg-name]` optional argument.  
"truthy" is a value that is not `F` or `N`.  
"falsey" is a value that is `F` or `N`.

**Expressions**

An evaluated expression returns a value. An expression is formed as `(operation [0..])` whereby the `operation` can be a native operation, program function, variable containing a function or lambda reference, or another expression evaluated as one of the aforementioned. Arguments can also be expressions.

**Functions**

Function declarations are only accepted at the top-level of a document or REPL interaction - they cannot be contained within expressions.  
They are declared as  
`(fn function-name […] [1..])` or `(fn function-name [1..])`  
where `[…]` is a vector of parameters (e.g. `[a b c]`), and `[1..]` is one or more expressions.

Anonymous functions, or lambdas, are defined by the syntax `#(operation [0..])` which constitutes one expression.

**Short-circuited control structures**

`(if cond if-true [if-false])`  
Returns `if-true` if `cond` is truthy, or `if-false` or `N` if `cond` is falsey.  
`if-true` and `if-false` are never mutually evaluated.

### Benchmarks

As of 5th August 2020. Most using [`hyperfine`](https://github.com/sharkdp/hyperfine) with `3` warmups, `10` runs.

| What   | C    | Kuan  | NodeJS | Wren   | Python3 pypy | Clojure            | C#                |
| ------ | ---- | ----- | ------ | ------ | ------------ | ------------------ | ----------------- |
| fib 35 | 35ms | 632ms | 168ms  | 1327ms | 250ms        | 2994ms, 143ms warm | 2630ms, 95ms warm |

**Kuan / Clojure (almost)**

```clj
(fn fib [n]
  (if (< n 2) n
    (+ (fib (- n 1))
       (fib (- n 2)))))
(println (fib 35))
```

**C**

```c
#include "stdio.h"
double fib (double n) {
  return n < 2 ? n : fib(n - 1) + fib(n - 2);
}
int main () {
  printf("%f\n", fib(35));
}
```

**Wren**

```csharp
class Fib {
  static get(n) {
    if (n < 2) return n
    return get(n - 1) + get(n - 2)
  }
}
System.print(Fib.get(35))
```

**Python3**

```py3
def fib(n):
  return n if n < 2 else fib(n - 1) + fib(n - 2)
print(fib(35))
```

**C#**

```csharp
static int fib (int n) => n < 2 ? n : fib(n - 1) + fib(n - 2);
static void Main () => Console.WriteLine(fib(35));```