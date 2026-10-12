# C in the Terminal — Course Syllabus

Reference document for the CLI course. Everything built later (exercises, checker, CLI tool) should trace back to this file.

## 1. Purpose

Teach the 20–30% of C that carries 80–90% of everyday C programming, entirely in a terminal, entirely hands-on.

A learner who finishes can:

- write, compile and run a multi-file C program of a few hundred lines;
- read a compiler error or sanitizer report and fix the cause;
- use pointers, arrays, strings and structs without guessing;
- allocate and release heap memory without leaks or use-after-free;
- read and write files and handle command-line arguments;
- read unfamiliar C code and follow what it does.

The course does not aim to produce C professionals. Anything rarely needed in a first year of writing C is left out (section 6).

## 2. Assumptions

| Item | Choice |
|---|---|
| Learner | New to C. Comfortable with a terminal and a text editor. Prior programming experience helps but is not required. |
| Language standard | C17 (`-std=c17`) |
| Compiler | `clang` or `gcc` |
| Standard flags | `-std=c17 -Wall -Wextra -Wpedantic -g` |
| Memory-safety flags | `-fsanitize=address,undefined`, switched on from module 4 |
| Platforms | macOS and Linux (Windows via WSL) |
| Size (estimate) | 13 modules, about 85 exercises, 15–20 hours |

## 3. Teaching principles

1. **Code first.** Each concept gets at most one screen of explanation, then an exercise.
2. **The compiler is the teacher.** Learners read real error messages from exercise 1 and are taught how to read them.
3. **One new idea per exercise.** Exercises are small; difficulty rises inside a module, then resets at the next.
4. **Memory is shown, not described.** Addresses, sizes and layouts are printed and inspected from module 4 onwards.
5. **Mistakes are made on purpose.** Each classic C bug (off-by-one, missing `\0`, dangling pointer, leak) appears as an exercise where the learner triggers it, sees the sanitizer report, and fixes it.
6. **Every module ends in a small working program**, and the last module assembles them into one project.

## 4. Exercise types

| Type | What the learner does | Passes when |
|---|---|---|
| `fix` | Repair code that fails to compile or warns | It compiles with zero warnings |
| `complete` | Fill in a `TODO` in a working skeleton | Output or tests match |
| `predict` | Read code and type the expected output before running it | The prediction matches |
| `debug` | Find a runtime bug in code that compiles cleanly | Sanitizers are silent and tests pass |
| `build` | Write a small program from a specification | Tests on stdin/stdout/exit code pass |

Every exercise has a short brief, progressive hints (up to three), and a reference solution unlocked after passing.

## 5. Modules

### Module 0 — The toolchain
*Goal: turn a `.c` file into a running program and understand what happened.*

- Source file → compiler → executable; `cc file.c -o prog`, `./prog`
- Anatomy of `main`, `#include <stdio.h>`, `return 0`, exit codes (`echo $?`)
- Warning flags and why they are always on
- Reading an error: file, line, column, message, caret

Exercises (4): fix a missing semicolon; fix a missing include; make a program return a given exit code; predict output.
**Mini-program:** `hello` that prints a banner and exits with status 0.

### Module 1 — Values, types and output
*Goal: store data and print it correctly.*

- `int`, `double`, `char`, `bool` (`<stdbool.h>`); `size_t` introduced by name
- Declaration, initialisation, `const`; uninitialised variables hold garbage
- `printf` format specifiers: `%d %f %c %s %zu %p`, width and precision
- Arithmetic, integer division, `%`, operator precedence
- Implicit conversion and explicit casts; `sizeof`
- Integer overflow as a first look at undefined behaviour

Exercises (8).
**Mini-program:** temperature conversion table.

### Module 2 — Control flow
*Goal: make decisions and repeat work.*

- `if` / `else if` / `else`; comparison and logical operators; short-circuiting
- Truthiness: zero is false, everything else is true
- `while`, `for`, `do … while`; `break`, `continue`
- `switch`, `case`, fall-through, `default`
- `=` versus `==`

Exercises (8).
**Mini-program:** FizzBuzz, then a prime lister.

### Module 3 — Functions
*Goal: split a program into named, reusable pieces.*

- Definition, parameters, return type, `void`
- Prototypes: declaration versus definition, and why order matters
- Pass by value: the function gets a copy
- Scope and lifetime of local variables; why globals are avoided
- Recursion, briefly (factorial, call stack picture)

Exercises (7).
**Mini-program:** a small calculator built from one function per operation.

### Module 4 — Arrays
*Goal: handle a fixed number of values, and meet C's lack of bounds checking.*

- Declaration, initialisers, indexing from zero
- Looping over an array; `sizeof(a) / sizeof(a[0])`
- Out-of-bounds access: no error, undefined behaviour; AddressSanitizer introduced here
- Passing arrays to functions: the size must travel separately
- Two-dimensional arrays, briefly

Exercises (7).
**Mini-program:** statistics (min, max, mean) over an array of numbers.

### Module 5 — Pointers
*Goal: understand that a pointer is an address, and use it deliberately. This is the centre of the course.*

- Memory as numbered boxes; `&` (address of) and `*` (dereference)
- Declaring pointers; `NULL`; never dereference an uninitialised pointer
- Output parameters: `swap(int *a, int *b)`
- Arrays and pointers: `a[i]` is `*(a + i)`; array-to-pointer decay
- Pointer arithmetic in units of the pointed-to type
- `const int *` versus `int *const`
- Returning a pointer to a local variable, and why it breaks

Exercises (10).
**Mini-program:** in-place array reversal and `swap`, written with pointers only.

### Module 6 — Strings
*Goal: work with text, knowing a string is a `char` array ending in `'\0'`.*

- String literals versus `char[]` versus `char *`
- The terminator, and what happens without it
- `<string.h>`: `strlen`, `strcmp`, `strchr`, `strstr`; safe copying with `snprintf`
- Why `gets` and unbounded `strcpy` / `scanf("%s")` are dangerous
- Reading a line with `fgets`; stripping the newline
- Text to number with `strtol` / `strtod`, with error checking
- `<ctype.h>`: `isdigit`, `isalpha`, `toupper`
- Command-line arguments: `argc`, `argv`

Exercises (10).
**Mini-program:** `wc`-style counter for lines, words and characters on stdin.

### Module 7 — Structs, enums and typedef
*Goal: model real things by grouping data.*

- `struct` definition, initialisation, designated initialisers, `.` access
- Pointers to structs and `->`
- Passing structs by value versus by pointer
- Arrays of structs; nested structs
- `enum` for named constants; `typedef` for readable type names

Exercises (7).
**Mini-program:** an in-memory contact list with a fixed capacity.

### Module 8 — Dynamic memory
*Goal: ask for memory at run time and give it back correctly.*

- Stack versus heap; lifetime
- `malloc`, `calloc`, `realloc`, `free`; `sizeof *p` idiom; checking for `NULL`
- Ownership: every allocation has exactly one owner responsible for freeing it
- The four classic bugs, each triggered and fixed: leak, use-after-free, double free, heap overflow
- Reading an AddressSanitizer / LeakSanitizer report
- A growable array (pointer, length, capacity)
- A singly linked list as the first self-referential struct

Exercises (10).
**Mini-program:** a growable `int` vector with `push`, `get` and `free`.

### Module 9 — Files and errors
*Goal: persist data and fail cleanly.*

- `stdin`, `stdout`, `stderr`; redirection and pipes from the shell
- `fopen` modes, `fclose`, checking for `NULL`
- Text I/O: `fgets`, `fprintf`, `fscanf` (with return-value checks)
- Binary I/O in brief: `fread`, `fwrite`
- C's error model: return codes, `errno`, `perror`, `EXIT_FAILURE`
- Cleaning up on every exit path

Exercises (7).
**Mini-program:** a `cat` / `grep`-lite that reads a file named on the command line.

### Module 10 — Multi-file programs
*Goal: organise code the way real C projects do.*

- Header (`.h`) versus source (`.c`); what belongs in each
- Include guards; `#include "x.h"` versus `<x.h>`
- Compiling and linking as separate steps; object files; reading a linker error
- `static` for file-private functions; `extern` in brief
- The preprocessor in practice: `#define` constants, simple macros and their pitfalls, `#ifdef`
- A minimal `Makefile`: targets, dependencies, variables, `make clean`

Exercises (6).
**Mini-program:** the module 8 vector split into `vec.h`, `vec.c` and `main.c` with a Makefile.

### Module 11 — Debugging and undefined behaviour
*Goal: find bugs methodically instead of by trial and error.*

- A working catalogue of undefined behaviour met so far, and why "it worked on my machine" means nothing
- `assert` for assumptions
- Debugger basics (`lldb` / `gdb`): breakpoint, step, print, backtrace
- Diagnosing a segfault from its backtrace
- Print-debugging to `stderr`, done properly

Exercises (5), all of type `debug`.

### Module 12 — Capstone
*Goal: build one complete command-line tool, using every module.*

A task tracker (`todo add "…"`, `todo list`, `todo done 3`, `todo rm 3`) that:

- parses `argv` (modules 2, 6);
- stores tasks as structs in a growable array (modules 7, 8);
- saves to and loads from a file (module 9);
- is split across several files with a Makefile (module 10);
- runs clean under the sanitizers (modules 8, 11);
- reports errors on `stderr` with non-zero exit codes (module 9).

Delivered in about six guided milestones, each checked automatically.

## 6. Deliberately left out

| Topic | Reason |
|---|---|
| Bitwise operators, bit fields | Needed for systems and embedded work; an optional appendix at most |
| Function pointers | Useful, but only essential for callbacks such as `qsort`; optional appendix |
| Unions, `goto`, `setjmp`/`longjmp` | Rare in beginner code |
| Variadic functions, `_Generic`, compound literals | Niche |
| Variable-length arrays | Discouraged in practice |
| Threads, atomics, signals | A separate subject |
| Wide characters, Unicode, locales | A separate subject |
| The full type zoo (`long long`, fixed-width, `float`, qualifiers such as `volatile`, `restrict`) | Mentioned only where they appear naturally |
| `scanf` in depth | `fgets` + `strtol` is safer and is what the course teaches |
| Platform APIs (POSIX, sockets, processes) | Outside the language itself |
| Algorithms and data structures beyond a vector and a linked list | A separate course |

## 7. Dependency order

```
0 Toolchain → 1 Values → 2 Control flow → 3 Functions → 4 Arrays → 5 Pointers
                                                                      │
                          ┌───────────────────────────────────────────┤
                          ▼                                           ▼
                      6 Strings                                  7 Structs
                          └──────────────┬────────────────────────────┘
                                         ▼
                                8 Dynamic memory → 9 Files → 10 Multi-file → 11 Debugging → 12 Capstone
```

The course is linear by default. Modules 6 and 7 can be taken in either order.

## 8. Sources consulted

- Beej's Guide to C Programming — chapter order (core chapters 2–12 map closely onto modules 0–8): https://beej.us/guide/bgc/html/split/
- CS50x — week order for its C portion (C, Arrays, Algorithms, Memory, Data Structures): https://cs50.harvard.edu/x/
- Rustlings — exercise format, watch mode, hints, progress list: https://rustlings.rust-lang.org/usage
- clings (Rustlings-style C exercises): https://github.com/codebysher/clings and https://pypi.org/project/clings/
- r/C_Programming learning resources: https://red.applefritter.com/r/C_Programming/wiki/index/learning
- Common pointer errors: https://dev.to/wakeup_flower_8591a6cb6a9/common-mistakes-in-pointers-c-language-2fd4
