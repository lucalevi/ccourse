# ccourse

**Learn C by writing C, entirely in your terminal.**

`ccourse` is an interactive, hands-on C course in the spirit of [Rustlings](https://rustlings.rust-lang.org/). You get a broken or unfinished C file, you fix it in your own editor, and the moment you save, `ccourse` compiles it, runs it and tells you whether it passes.

The course teaches the roughly 20% of C that delivers 80% of the results: the syntax, the memory model, and the habits needed to write small, correct programs. It is not meant to make you a C guru. The full plan is in [SYLLABUS.md](SYLLABUS.md).

```
hello1 · 00_toolchain · exercise 1 of 36 · fix
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 0/36 done

Open in your editor: exercises/00_toolchain/hello1.c

✗ The program does not compile

exercises/00_toolchain/hello1.c:22:26: error: expected ';' after expression
   22 |     printf("Hello, C!\n")
      |                          ^
      |                          ;

h hint · l list · r re-run · x reset · q quit
```

## Features

- **Watch mode.** Save the file and the result appears at once; there is nothing to run by hand.
- **The compiler is the teacher.** Real compiler messages, with warnings treated as failures from the first exercise.
- **Sanitizers from module 4.** Out-of-bounds access and other undefined behaviour are caught with AddressSanitizer and UndefinedBehaviorSanitizer, and you learn to read their reports.
- **Five exercise types.** Fix a compile error, complete a `TODO`, predict a program's output, debug a runtime bug, or build a program from a specification.
- **Progressive hints and reference solutions.** Hints are revealed one at a time; the solution is shown once you pass.
- **Saved progress.** Pick up where you left off.

## Status

| Module | Topic | State |
|---|---|---|
| 0 | The toolchain | ready |
| 1 | Values, types and output | ready |
| 2 | Control flow | ready |
| 3 | Functions | ready |
| 4 | Arrays | ready |
| 5 | Pointers | planned |
| 6 | Strings | planned |
| 7 | Structs, enums and typedef | planned |
| 8 | Dynamic memory | planned |
| 9 | Files and errors | planned |
| 10 | Multi-file programs | planned |
| 11 | Debugging and undefined behaviour | planned |
| 12 | Capstone: a command-line task tracker | planned |

## Requirements

- [Rust](https://rustup.rs/) to build the tool
- A C compiler reachable as `cc` (clang or gcc); set the `CC` environment variable to use another
- macOS or Linux (Windows via WSL)
- A text editor of your choice

## Getting started

```
git clone <this repository>
cd ccourse
cargo build --release
./target/release/ccourse
```

Leave `ccourse` running, open the file it names in your editor (in another terminal or a split), edit, and save.

| Key | Action |
|---|---|
| `n` | next exercise (once the current one passes) |
| `h` | show the next hint |
| `l` | list all exercises and jump to one |
| `r` | check again without saving |
| `x` | reset the file to its original state |
| `q` | quit |

Your working copies are created in `exercises/` and your progress is kept in `.ccourse/`. Delete both to start over. More detail, including how exercises are written and verified, is in [GUIDE.md](GUIDE.md).

## Contributing

Exercises are plain C files plus an entry in `course/info.toml`. After adding or changing any, run:

```
./target/release/ccourse verify
```

It checks that every original exercise fails and every reference solution passes. Please follow the plan and the list of deliberate omissions in [SYLLABUS.md](SYLLABUS.md).

## License

This work is licensed under the [Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License](https://creativecommons.org/licenses/by-nc-sa/4.0/). See [LICENSE](LICENSE).

You may share and adapt it with attribution, for non-commercial purposes, and under the same license.
