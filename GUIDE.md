# ccourse

A hands-on C course that runs in the terminal. The plan is in [SYLLABUS.md](SYLLABUS.md).

## Requirements

- Rust (to build the tool)
- A C compiler available as `cc` (clang or gcc). Set `CC` to use another one.

## Taking the course

```
cargo build --release
./target/release/ccourse
```

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

After any change, run `ccourse verify`. It checks that every original fails and every solution passes.
