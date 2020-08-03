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
- Easy imports (NYA)

**Anti-features / trade-offs / corner-cutting**  
- No program errors
- Leaked memory with inappropriate use

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

### Benchmarks

As of 3rd August 2020. Using [`hyperfine`](https://github.com/sharkdp/hyperfine) with a warmup of `3`, `10` runs.

| What   | C       | Kuan      | Wren      | Python3    | Clojure CLI | Clojure REPL |
| ------ | ------- | --------- | --------- | ---------- | ----------- | ------------ |
| fib 35 | 35 ±1ms | 669 ±17ms | 1327 ±4ms | 2713 ±75ms | 2994 ±90ms  | 143ms        |

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