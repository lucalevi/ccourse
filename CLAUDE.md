# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

`ccourse` is a terminal-only, hands-on C course (a "Rustlings for C"). A CLI written in C runs in watch mode: it re-compiles and re-checks the learner's current exercise every time they save the file. `SYLLABUS.md` is the authoritative plan (13 modules, 0–12, Pareto-style: only the ~20% of C that gives ~80% of the value). New exercises must follow it, including its "deliberately left out" list. `README.md` is the GitHub front page; `GUIDE.md` has the learner usage and exercise-authoring notes.

Status: modules 0–4 (36 exercises) are written; modules 5–12 are not.

## Commands

```
make                       # builds build/ccourse
./build/ccourse            # watch mode (needs an interactive TTY); run from repo root or below
make verify                # author check: every original must FAIL, every solution must PASS, every exercise needs hints
make install               # copies ccourse to $(PREFIX)/bin, default ~/.local/bin
make BUILD=/tmp/ccourse-asan CFLAGS='-g -fsanitize=address,undefined'   # sanitizer build of the tool itself
```

The tool is built with `-std=c17 -Wall -Wextra -Wpedantic` and must compile with zero warnings, the same standard the exercises are held to. It uses POSIX only (fork/exec, poll, termios) and has no dependencies beyond libc.

There are no unit tests; `ccourse verify` is the test suite for course content. Run it after any change to `course/` or to `src/check.c`. After changing the tool, also run `verify` with the sanitizer build. `verify` creates `.ccourse/bin/` build output; learner state (`exercises/`, `.ccourse/`) is gitignored, so never run watch mode in the repo as a test (it rewrites `.ccourse/state.toml`); use a scratch copy of `course/`.

The C compiler is `cc` by default, overridable with `CC`, both for building the tool and for the exercises it compiles.

The tool was first written in Rust. That version may exist locally in the gitignored `rust-reference/` directory, as a reference only; it is not built or shipped.

## Architecture

C modules in `src/` (each a `.c` with its `.h`), plus the course content in `course/`:

- `course.c` — loads `course/info.toml` (found by walking up from the cwd), resolves the three path roles per exercise, and persists progress in `.ccourse/state.toml`.
- `check.c` — compile + run + compare. Used by both watch mode and `verify`. Child processes are started with fork/exec and attended with `poll`.
- `watch.c` — raw-mode terminal UI (termios): polls the working file's mtime every 200 ms, handles keys, renders. `main.c` dispatches to it or to `verify`.
- `toml.c` — reader for the TOML subset the course uses: comments, `[[tables]]`, single-line strings, integers, booleans, lists of strings. Anything else in `info.toml` (multi-line strings, inline tables, floats, `[table]`) is an error, as are unknown keys.
- `util.c` — growable strings (`Str`, `Strings`), file helpers, and the error convention: a function that returns `false`/`NULL` has first recorded the reason with `fail()`, and `main` prints `failure()`.

Ownership: functions returning `char *` hand the caller a string to `free`, unless the comment in the header says otherwise.

### Content layout (three copies of each exercise)

| Path | Role |
|---|---|
| `course/exercises/<dir>/<name>.c` | pristine original; source of truth for resets |
| `course/solutions/<dir>/<name>.c` | reference solution, shown to the learner after passing |
| `exercises/<dir>/<name>.c` | learner's working copy, created on first run, **the file being watched** |

`course/info.toml` defines order, `kind`, `sanitize`, `hints`, and `[[exercises.tests]]` (`args`, `stdin`, `stdout`, `exit`). Exercise order is the order in that file. Names must be unique (they key progress and the binary name).

### How an exercise is judged (`check.c`)

1. Compile with `-std=c17 -Wall -Wextra -Wpedantic -g` (plus `-fsanitize=address,undefined` when `sanitize = true`). **Any compiler output at all counts as failure**, since warnings must be fixed in this course.
2. Run each test with a 5 s time limit and 64 KiB output cap; compare exit code and (normalised) stdout.
3. `predict` exercises ignore `tests`: the learner writes the expected output between `/* PREDICT` and `*/` marker lines at the end of the .c file, and it is compared with the actual output.

## Conventions when writing exercises

- Each .c file opens with a comment that teaches the one new idea and ends with a `TASK` section; keep it to about one screen. The expected output is stated in that comment and must match the test in `info.toml`.
- Originals must fail in a way that produces a useful compiler or sanitizer message; `ccourse verify` fails an original that already passes.
- Every exercise needs 2–3 progressive hints, and the last should be close to the answer.
- `sanitize = true` from module 4 onward (per the syllabus).
- Exercises that deliberately provoke undefined behaviour must be `kind = "debug"` and still compile with zero warnings, so the sanitizer, not the compiler, is what catches it.
- A `predict` original has an empty `/* PREDICT` block; its solution has the filled-in one.
- Platform note: Apple clang's AddressSanitizer does not do leak detection, so module 8's leak exercises need a different mechanism than `detect_leaks`.

## Licensing

Split by content, see `NOTICE`: code (`src/`, `Makefile`) is PolyForm Noncommercial 1.0.0 (`LICENSE-CODE`); course content (`course/`, docs) is CC BY-NC-SA 4.0 (`LICENSE-CONTENT`). Do not add dependencies or copied code with licenses that conflict with non-commercial terms.
