# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

`ccourse` is a terminal-only, hands-on C course (a "Rustlings for C"). A Rust CLI runs in watch mode: it re-compiles and re-checks the learner's current exercise every time they save the file. `SYLLABUS.md` is the authoritative plan (13 modules, 0–12, Pareto-style: only the ~20% of C that gives ~80% of the value). New exercises must follow it, including its "deliberately left out" list. `README.md` is the GitHub front page; `GUIDE.md` has the learner usage and exercise-authoring notes.

Status: modules 0–4 (36 exercises) are written; modules 5–12 are not.

## Commands

```
cargo build --release
./target/release/ccourse            # watch mode (needs an interactive TTY); run from repo root or below
./target/release/ccourse verify     # author check: every original must FAIL, every solution must PASS, every exercise needs hints
cargo clippy --release
```

There are no Rust unit tests; `ccourse verify` is the test suite for course content. Run it after any change to `course/` or to `src/check.rs`. `verify` creates `.ccourse/bin/` build output; learner state (`exercises/`, `.ccourse/`) is gitignored.

The C compiler is `cc` by default, overridable with `CC`.

## Architecture

Three Rust modules in `src/`, plus the course content in `course/`:

- `course.rs` — loads `course/info.toml` (found by walking up from the cwd), resolves the three path roles per exercise, and persists progress in `.ccourse/state.toml`.
- `check.rs` — compile + run + compare. Used by both watch mode and `verify`.
- `watch.rs` — raw-mode terminal UI: polls the working file's mtime every 200 ms, handles keys, renders. `main.rs` dispatches to it or to `verify`.

### Content layout (three copies of each exercise)

| Path | Role |
|---|---|
| `course/exercises/<dir>/<name>.c` | pristine original; source of truth for resets |
| `course/solutions/<dir>/<name>.c` | reference solution, shown to the learner after passing |
| `exercises/<dir>/<name>.c` | learner's working copy, created on first run, **the file being watched** |

`course/info.toml` defines order, `kind`, `sanitize`, `hints`, and `[[exercises.tests]]` (`args`, `stdin`, `stdout`, `exit`). Exercise order is the order in that file. Names must be unique (they key progress and the binary name).

### How an exercise is judged (`check.rs`)

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

Split by content, see `NOTICE`: code (`src/`, Cargo files) is PolyForm Noncommercial 1.0.0 (`LICENSE-CODE`); course content (`course/`, docs) is CC BY-NC-SA 4.0 (`LICENSE-CONTENT`). Do not add dependencies or copied code with licenses that conflict with non-commercial terms.
