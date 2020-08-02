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