#include "check.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/* A runaway program is cut off after this much output or this much time. */
#define OUTPUT_LIMIT ((size_t)64 * 1024)
#define TIME_LIMIT_MS 5000
/* The compiler is trusted to stop by itself, but not to be brief. */
#define COMPILER_OUTPUT_LIMIT ((size_t)1024 * 1024)
#define NO_TIME_LIMIT (-1)

static Outcome failed(const char *title, char *detail) {
    return (Outcome){.passed = false, .title = xstrdup(title), .detail = detail};
}

void outcome_free(Outcome *outcome) {
    free(outcome->output);
    free(outcome->title);
    free(outcome->detail);
    *outcome = (Outcome){0};
}

/* ── Running a program ────────────────────────────────────────────────────── */

typedef struct {
    Str out;
    Str err;
    /* False if a signal ended the program; then `signal` says which. */
    bool exited;
    int code;
    int signal;
    bool timed_out;
} Run;

static void run_free(Run *run) {
    str_free(&run->out);
    str_free(&run->err);
}

static void close_fd(int *fd) {
    if (*fd >= 0) {
        close(*fd);
        *fd = -1;
    }
}

static long long now_ms(void) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (long long)now.tv_sec * 1000 + now.tv_nsec / 1000000;
}

/* Starts a program with pipes to its stdin, stdout and stderr, in that order
 * in `fds`. Returns -1 and sets errno if it cannot be started. */
static pid_t spawn(const char *const argv[], const char *cwd, int fds[3]) {
    /* Four pipes, each a read end then a write end: the child's stdin, stdout
     * and stderr, and one on which the child reports that exec failed. */
    int in[2] = {-1, -1}, out[2] = {-1, -1}, err[2] = {-1, -1}, report[2] = {-1, -1};
    pid_t pid = -1;
    if (pipe(in) == 0 && pipe(out) == 0 && pipe(err) == 0 && pipe(report) == 0) {
        pid = fork();
    }
    if (pid == 0) {
        /* The child. This pipe end closes by itself if exec succeeds. */
        fcntl(report[1], F_SETFD, FD_CLOEXEC);
        signal(SIGPIPE, SIG_DFL);
        if ((cwd == NULL || chdir(cwd) == 0) && dup2(in[0], STDIN_FILENO) >= 0 &&
            dup2(out[1], STDOUT_FILENO) >= 0 && dup2(err[1], STDERR_FILENO) >= 0) {
            close(in[0]), close(in[1]), close(out[0]), close(out[1]);
            close(err[0]), close(err[1]), close(report[0]);
            /* Silences a macOS allocator notice that AddressSanitizer triggers. */
            setenv("MallocNanoZone", "0", 1);
            execvp(argv[0], (char *const *)argv);
        }
        int reason = errno;
        ssize_t ignored = write(report[1], &reason, sizeof reason);
        (void)ignored;
        _exit(127);
    }

    int reason = errno;
    close_fd(&in[0]), close_fd(&out[1]), close_fd(&err[1]), close_fd(&report[1]);
    if (pid > 0) {
        /* Nothing arrives if exec succeeded. */
        ssize_t n;
        do {
            n = read(report[0], &reason, sizeof reason);
        } while (n < 0 && errno == EINTR);
        if (n == (ssize_t)sizeof reason) {
            waitpid(pid, NULL, 0);
            pid = -1;
        }
    }
    close_fd(&report[0]);
    if (pid < 0) {
        close_fd(&in[1]), close_fd(&out[0]), close_fd(&err[0]);
        errno = reason;
        return -1;
    }
    fds[0] = in[1], fds[1] = out[0], fds[2] = err[0];
    return pid;
}

/* Moves what is waiting in a pipe into `into`, and closes the pipe once it is
 * finished or the limit is reached. */
static void drain(int *fd, Str *into, size_t limit) {
    char buf[4096];
    size_t room = limit - into->len < sizeof buf ? limit - into->len : sizeof buf;
    ssize_t n = read(*fd, buf, room);
    if (n > 0) {
        str_addn(into, buf, (size_t)n);
    }
    if (n == 0 || (n < 0 && errno != EINTR && errno != EAGAIN) || into->len >= limit) {
        close_fd(fd);
    }
}

static int wait_for(pid_t pid) {
    int status = 0;
    while (waitpid(pid, &status, 0) < 0 && errno == EINTR) {
    }
    return status;
}

/* Feeds `input` to a started program, gathers what it prints and waits for it
 * to end, stopping it if it takes longer than `time_limit` milliseconds. */
static void attend(pid_t pid, int fds[3], const char *input, size_t limit, int time_limit, Run *run) {
    int in = fds[0], out = fds[1], err = fds[2];
    size_t left = strlen(input);
    long long deadline = now_ms() + time_limit;
    bool limited = time_limit != NO_TIME_LIMIT;

    /* A program that does not read its input must not hold us up. */
    fcntl(in, F_SETFL, O_NONBLOCK);
    if (left == 0) {
        close_fd(&in);
    }
    while (out >= 0 || err >= 0) {
        long long remaining = deadline - now_ms();
        if (limited && remaining <= 0) {
            run->timed_out = true;
            break;
        }
        struct pollfd polled[3] = {{in, POLLOUT, 0}, {out, POLLIN, 0}, {err, POLLIN, 0}};
        int ready = poll(polled, 3, limited ? (int)remaining : -1);
        if (ready < 0 && errno != EINTR) {
            break;
        }
        if (ready <= 0) {
            continue;
        }
        if (polled[0].revents != 0) {
            ssize_t n = write(in, input, left);
            if (n > 0) {
                input += n;
                left -= (size_t)n;
            }
            if (left == 0 || (n < 0 && errno != EINTR && errno != EAGAIN)) {
                close_fd(&in);
            }
        }
        if (polled[1].revents != 0) {
            drain(&out, &run->out, limit);
        }
        if (polled[2].revents != 0) {
            drain(&err, &run->err, limit);
        }
    }
    close_fd(&in), close_fd(&out), close_fd(&err);

    int status = 0;
    if (!limited) {
        status = wait_for(pid);
    } else {
        while (waitpid(pid, &status, WNOHANG) == 0) {
            if (run->timed_out || now_ms() >= deadline) {
                run->timed_out = true;
                kill(pid, SIGKILL);
                status = wait_for(pid);
                break;
            }
            sleep_ms(5);
        }
    }
    run->exited = WIFEXITED(status);
    run->code = run->exited ? WEXITSTATUS(status) : 0;
    run->signal = WIFSIGNALED(status) ? WTERMSIG(status) : 0;
}

/* Runs the compiled exercise as one test asks. Sets errno if it cannot start. */
static bool run_test(const char *bin, const Test *test, Run *run) {
    const char **argv = xmalloc((test->args.count + 2) * sizeof *argv);
    argv[0] = bin;
    for (size_t i = 0; i < test->args.count; i++) {
        argv[i + 1] = test->args.items[i];
    }
    argv[test->args.count + 1] = NULL;

    *run = (Run){0};
    int fds[3];
    pid_t pid = spawn(argv, NULL, fds);
    int reason = errno;
    free(argv);
    if (pid < 0) {
        errno = reason;
        return false;
    }
    attend(pid, fds, test->input ? test->input : "", OUTPUT_LIMIT, TIME_LIMIT_MS, run);
    return true;
}

/* ── Compiling ────────────────────────────────────────────────────────────── */

/* Builds the exercise. Returns the path of the program for the caller to
 * free, or NULL with the reason in `failure`. */
static char *compile(const Course *course, const Exercise *ex, const char *source, Outcome *failure_out) {
    char *bin = course_bin(course, ex);
    if (!make_parent_dirs(bin)) {
        free(bin);
        *failure_out = failed("Could not create the build directory", format("%s\n", failure()));
        return NULL;
    }
    const char *compiler = getenv("CC");
    if (compiler == NULL || *compiler == '\0') {
        compiler = "cc";
    }

    const char *argv[16];
    size_t n = 0;
    argv[n++] = compiler;
    argv[n++] = "-std=c17";
    argv[n++] = "-Wall";
    argv[n++] = "-Wextra";
    argv[n++] = "-Wpedantic";
    argv[n++] = "-g";
    argv[n++] = "-fdiagnostics-color=always";
    if (ex->sanitize) {
        argv[n++] = "-fsanitize=address,undefined";
        argv[n++] = "-fno-omit-frame-pointer";
    }
    /* Compiling from the course directory keeps the paths in messages short. */
    argv[n++] = course_relative(course, source);
    argv[n++] = "-o";
    argv[n++] = bin;
    argv[n] = NULL;

    int fds[3];
    pid_t pid = spawn(argv, course->root, fds);
    if (pid < 0) {
        *failure_out = failed(
            "Could not start the C compiler",
            format("Tried to run `%s`: %s\nInstall clang or gcc, or set the CC environment variable.\n", compiler,
                   strerror(errno)));
        free(bin);
        return NULL;
    }
    Run run = {0};
    attend(pid, fds, "", COMPILER_OUTPUT_LIMIT, NO_TIME_LIMIT, &run);
    char *messages = str_take(&run.err);
    bool succeeded = run.exited && run.code == 0;
    run_free(&run);

    if (!succeeded) {
        *failure_out = failed("The program does not compile", messages);
    } else if (!is_blank(messages)) {
        *failure_out = failed("The program compiles, but with warnings. Fix those too", messages);
    } else {
        free(messages);
        return bin;
    }
    free(bin);
    return NULL;
}

/* ── Comparing ────────────────────────────────────────────────────────────── */

static void trim_end(char *text) {
    size_t len = strlen(text);
    while (len > 0 && isspace((unsigned char)text[len - 1])) {
        len--;
    }
    text[len] = '\0';
}

static void trim(char *text) {
    trim_end(text);
    char *start = text;
    while (isspace((unsigned char)*start)) {
        start++;
    }
    memmove(text, start, strlen(start) + 1);
}

/* Output as lines, ignoring trailing spaces and trailing blank lines. */
static Strings normalise(const char *text) {
    Strings lines = split_lines(text);
    for (size_t i = 0; i < lines.count; i++) {
        trim_end(lines.items[i]);
    }
    while (lines.count > 0 && lines.items[lines.count - 1][0] == '\0') {
        free(lines.items[--lines.count]);
    }
    return lines;
}

/* Like `normalise`, but also forgiving of indentation and leading blank lines. */
static Strings trimmed(const char *text) {
    text += strspn(text, "\r\n");
    Strings lines = normalise(text);
    for (size_t i = 0; i < lines.count; i++) {
        trim(lines.items[i]);
    }
    return lines;
}

/* The number of leading lines that two texts have in common. */
static size_t lines_in_common(const Strings *a, const Strings *b) {
    size_t i = 0;
    while (i < a->count && i < b->count && strcmp(a->items[i], b->items[i]) == 0) {
        i++;
    }
    return i;
}

static bool same_output(const char *expected, const char *actual) {
    Strings a = normalise(expected);
    Strings b = normalise(actual);
    bool same = a.count == b.count && lines_in_common(&a, &b) == a.count;
    strings_free(&a);
    strings_free(&b);
    return same;
}

/* Sets text off from the surrounding explanation. */
static void add_block(Str *s, const char *text) {
    if (*text == '\0') {
        str_add(s, "  " DIM "│ (nothing)" RESET "\n");
        return;
    }
    Strings lines = split_lines(text);
    for (size_t i = 0; i < lines.count; i++) {
        str_addf(s, "  " DIM "│" RESET " %s\n", lines.items[i]);
    }
    strings_free(&lines);
}

/* The error output without the raw memory map that ends an AddressSanitizer
 * report. The caller frees it. */
static char *without_memory_dump(const char *report) {
    Strings lines = split_lines(report);
    Str kept = {0};
    for (size_t i = 0; i < lines.count && strncmp(lines.items[i], "Shadow bytes", 12) != 0; i++) {
        str_addf(&kept, "%s\n", lines.items[i]);
    }
    strings_free(&lines);
    return str_take(&kept);
}

static const char *signal_meaning(int signal) {
    switch (signal) {
    case SIGABRT: return " (abort)";
    case SIGFPE: return " (arithmetic error, such as division by zero)";
    case SIGBUS: return " (bus error: invalid memory access)";
    case SIGSEGV: return " (segmentation fault: invalid memory access)";
    default: return "";
    }
}

/* Describes how a run differs from what the test expects. Returns NULL if it
 * does not; otherwise the caller frees the text. */
static char *compare(const Run *run, const Test *test) {
    if (run->timed_out) {
        return format("The program was still running after %d seconds and was stopped.\n"
                      "Is there a loop that never ends?\n",
                      TIME_LIMIT_MS / 1000);
    }
    if (run->out.len >= OUTPUT_LIMIT) {
        return xstrdup("The program printed far too much and was stopped.\nIs there a loop that never ends?\n");
    }

    Str problem = {0};
    const char *printed = run->out.data ? run->out.data : "";
    if (!run->exited) {
        str_addf(&problem, "The program crashed: killed by signal %d%s.\n", run->signal, signal_meaning(run->signal));
    } else if (run->code != test->exit_code) {
        str_addf(&problem, "Expected exit code %d, but the program exited with %d.\n", test->exit_code, run->code);
    }
    /* A crash loses whatever output was still buffered, so comparing it would mislead. */
    if (test->output != NULL && run->exited && !same_output(test->output, printed)) {
        str_add(&problem, "Expected output:\n");
        add_block(&problem, test->output);
        str_add(&problem, "Your program printed:\n");
        add_block(&problem, printed);
    }
    if (problem.len == 0) {
        return NULL;
    }
    if (run->err.data != NULL && !is_blank(run->err.data)) {
        char *errors = without_memory_dump(run->err.data);
        str_add(&problem, "Error output of the program:\n");
        add_block(&problem, errors);
        free(errors);
    }
    return str_take(&problem);
}

/* ── Predictions ──────────────────────────────────────────────────────────── */

/* True if the line, apart from surrounding spaces, is exactly `marker`. */
static bool is_marker(const char *line, const char *marker) {
    size_t len = strlen(marker);
    while (isspace((unsigned char)*line)) {
        line++;
    }
    return strncmp(line, marker, len) == 0 && is_blank(line + len);
}

/* The text between the two PREDICT marker lines, or NULL if the opening one
 * is missing. The caller frees it. */
static char *prediction(const char *source) {
    Strings lines = split_lines(source);
    size_t i = 0;
    while (i < lines.count && !is_marker(lines.items[i], "/* PREDICT")) {
        i++;
    }
    if (i == lines.count) {
        strings_free(&lines);
        return NULL;
    }
    Str body = {0};
    for (i++; i < lines.count && !is_marker(lines.items[i], "*/"); i++) {
        str_addf(&body, "%s\n", lines.items[i]);
    }
    strings_free(&lines);
    return str_take(&body);
}

static Outcome check_prediction(const char *bin, const char *source) {
    static const Test plain_run = {0};
    Run run;
    if (!run_test(bin, &plain_run, &run)) {
        return failed("Could not run the program", format("%s\n", strerror(errno)));
    }
    char *printed = str_take(&run.out);
    run_free(&run);

    char *text = read_file(source);
    char *predicted_text = text ? prediction(text) : NULL;
    free(text);
    if (predicted_text == NULL) {
        free(printed);
        return failed("The PREDICT markers are missing",
                      xstrdup("The file must end with a line `/* PREDICT`, your prediction, then a line `*/`.\n"
                              "Press x to reset the file if you removed them.\n"));
    }
    Strings predicted = trimmed(predicted_text);
    Strings actual = trimmed(printed);
    size_t common = lines_in_common(&predicted, &actual);
    free(predicted_text);

    Outcome outcome;
    if (predicted.count == 0) {
        outcome = failed("Waiting for your prediction",
                         xstrdup("Read the code, then write exactly what it prints between the\n"
                                 "`/* PREDICT` and `*/` lines at the bottom of the file, and save.\n"));
    } else if (common == predicted.count && common == actual.count) {
        outcome = (Outcome){.passed = true, .output = printed};
        printed = NULL;
    } else if (common < predicted.count && common < actual.count) {
        outcome = failed("The prediction does not match",
                         format("Line %zu of your prediction is not what the program prints.\n", common + 1));
    } else if (predicted.count < actual.count) {
        outcome = failed("The prediction does not match",
                         format("Correct so far, but the program prints more lines than your %zu.\n", predicted.count));
    } else {
        outcome = failed("The prediction does not match",
                         format("The program prints only %zu line(s); your prediction has more.\n", actual.count));
    }
    free(printed);
    strings_free(&predicted);
    strings_free(&actual);
    return outcome;
}

/* ── Judging ──────────────────────────────────────────────────────────────── */

Outcome check(const Course *course, const Exercise *ex, const char *source) {
    Outcome outcome;
    char *bin = compile(course, ex, source, &outcome);
    if (bin == NULL) {
        return outcome;
    }
    if (ex->kind == KIND_PREDICT) {
        outcome = check_prediction(bin, source);
        free(bin);
        return outcome;
    }

    outcome = (Outcome){.passed = true};
    for (size_t i = 0; i < ex->test_count && outcome.passed; i++) {
        const Test *test = &ex->tests[i];
        Run run;
        if (!run_test(bin, test, &run)) {
            outcome_free(&outcome);
            outcome = failed("Could not run the program", format("%s\n", strerror(errno)));
            break;
        }
        char *problem = compare(&run, test);
        if (problem != NULL) {
            Str detail = {0};
            if (ex->test_count > 1) {
                str_addf(&detail, DIM "Test %zu of %zu" RESET "\n", i + 1, ex->test_count);
            }
            if (test->args.count > 0) {
                str_add(&detail, "Run with arguments:");
                for (size_t a = 0; a < test->args.count; a++) {
                    str_addf(&detail, " %s", test->args.items[a]);
                }
                str_add(&detail, "\n");
            }
            if (test->input != NULL && *test->input != '\0') {
                str_add(&detail, "Given this input:\n");
                add_block(&detail, test->input);
            }
            str_add(&detail, problem);
            free(problem);
            outcome_free(&outcome);
            outcome = failed("The program compiles, but does not behave as required", str_take(&detail));
        } else if (i == 0) {
            outcome.output = str_take(&run.out);
        }
        run_free(&run);
    }
    if (outcome.passed && outcome.output == NULL) {
        outcome.output = xstrdup("");
    }
    free(bin);
    return outcome;
}
