# ccourse

A hands-on C course that runs in the terminal. The plan is in [SYLLABUS.md](SYLLABUS.md).

## Requirements

- A C compiler available as `cc` (clang or gcc). Set `CC` to use another one.
- `make`

The tool is written in C, so the compiler you learn with is also the one that builds it.

## Taking the course

```
make
./build/ccourse
```

To have `ccourse` on your `PATH`, run `make install`. It copies the program to `~/.local/bin`; choose another place with `make install PREFIX=/usr/local`. `ccourse` looks for the course in the current directory and its parents, so start it from inside this repository.

Leave it running and open the file it names in your editor, in a second terminal or a split. Every time you save, the exercise is compiled and checked again.

| Key | Action |
|---|---|
| `n` | next exercise (once the current one passes) |
| `h` | show the next hint |
| `l` | list all exercises and jump to one |
| `r` | check again without saving |
| `x` | reset the file to its original state |
| `q` | quit |

Your copies of the exercises live in `exercises/`, and your progress in `.ccourse/`. Both are created on first run. Delete them to start over.

## Writing exercises

- `course/info.toml` lists the exercises in order, with their hints and tests.
- `course/exercises/<dir>/<name>.c` is the original handed to the learner.
- `course/solutions/<dir>/<name>.c` is the reference solution.

After any change, run `make verify` (or `ccourse verify`). It checks that every original fails and every solution passes.

`course/info.toml` is read by a small TOML reader in `src/toml.c` that knows only what the course needs: comments, `[[exercises]]` and `[[exercises.tests]]`, and values that are single-line strings, whole numbers, `true`/`false` or lists of strings. A misspelt key is reported as an error with its line number.
