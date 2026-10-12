#include "watch.h"

#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

#include "check.h"

#define POLL_MS 200
#define MAX_DETAIL_LINES 60
#define BAR_WIDTH 30

/* ── The terminal ─────────────────────────────────────────────────────────── */

/* Keys that are not a single character. */
enum { KEY_NONE = 0, KEY_CTRL_C = 3, KEY_ENTER = '\r', KEY_ESC = 27, KEY_UP = 1000, KEY_DOWN };

static struct termios saved_terminal;
static bool in_raw_mode;

/* Set by signal handlers and looked at by the loops below. */
static volatile sig_atomic_t resized;
static volatile sig_atomic_t stopped;

static void on_resize(int signal) {
    (void)signal;
    resized = 1;
}

static void on_stop(int signal) {
    (void)signal;
    stopped = 1;
}

/* Restores the terminal however watch mode ends. */
static void leave_raw_mode(void) {
    if (in_raw_mode) {
        in_raw_mode = false;
        fputs("\x1b[?25h", stdout);
        fflush(stdout);
        tcsetattr(STDIN_FILENO, TCSANOW, &saved_terminal);
    }
}

/* Raw mode delivers each key at once, without echo, and hides the cursor. */
static bool enter_raw_mode(void) {
    if (tcgetattr(STDIN_FILENO, &saved_terminal) != 0) {
        fail("cannot read the terminal settings: %s", strerror(errno));
        return false;
    }
    struct termios raw = saved_terminal;
    raw.c_iflag &= ~(tcflag_t)(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(tcflag_t)OPOST;
    raw.c_lflag &= ~(tcflag_t)(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) != 0) {
        fail("cannot change the terminal settings: %s", strerror(errno));
        return false;
    }
    in_raw_mode = true;
    atexit(leave_raw_mode);

    struct sigaction action = {0};
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;
    action.sa_handler = on_resize;
    sigaction(SIGWINCH, &action, NULL);
    action.sa_handler = on_stop;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGHUP, &action, NULL);
    sigaction(SIGINT, &action, NULL);

    fputs("\x1b[?25l", stdout);
    return true;
}

/* Waits up to `timeout` ms for a byte from the keyboard. Returns -1 if none
 * came, which also happens when a signal arrives. */
static int read_byte(int timeout) {
    struct pollfd keyboard = {STDIN_FILENO, POLLIN, 0};
    if (poll(&keyboard, 1, timeout) <= 0) {
        return -1;
    }
    unsigned char byte;
    ssize_t n = read(STDIN_FILENO, &byte, 1);
    if (n == 1) {
        return byte;
    }
    if (n == 0 || (errno != EINTR && errno != EAGAIN)) {
        /* The terminal is gone. */
        stopped = 1;
    }
    return -1;
}

/* Waits up to `timeout` ms (for ever if negative) for a key press. Returns
 * KEY_NONE if none came, which also happens when a signal arrives. */
static int read_key(int timeout) {
    int byte = read_byte(timeout);
    if (byte < 0) {
        return KEY_NONE;
    }
    if (byte == '\n') {
        return KEY_ENTER;
    }
    if (byte != KEY_ESC) {
        return byte;
    }
    /* An arrow key arrives as ESC [ A; the Esc key as ESC alone. */
    byte = read_byte(25);
    if (byte < 0) {
        return KEY_ESC;
    }
    if (byte != '[' && byte != 'O') {
        return KEY_NONE;
    }
    do {
        byte = read_byte(25);
    } while (byte >= 0x20 && byte < 0x40);
    return byte == 'A' ? KEY_UP : byte == 'B' ? KEY_DOWN : KEY_NONE;
}

static bool is_quit(int key) {
    return key == 'q' || key == KEY_CTRL_C;
}

static size_t terminal_rows(void) {
    struct winsize size;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) != 0 || size.ws_row == 0) {
        return 24;
    }
    return size.ws_row;
}

/* Replaces the screen contents. Raw mode needs explicit carriage returns. */
static void show(const char *text) {
    fputs("\x1b[2J\x1b[3J\x1b[H", stdout);
    for (; *text != '\0'; text++) {
        if (*text == '\n') {
            putchar('\r');
        }
        putchar(*text);
    }
    fflush(stdout);
}

/* ── The session ──────────────────────────────────────────────────────────── */

typedef struct {
    const Course *course;
    State state;
    size_t index;
    Outcome outcome;
    size_t hints_shown;
    /* A one-off message, shown until the next key press or save. */
    const char *note;
} Session;

static const Exercise *current(const Session *s) {
    return &s->course->exercises[s->index];
}

/* When the watched file was last saved, or -1 if it is missing. */
static long long modified(const Session *s) {
    char *path = course_working(s->course, current(s));
    struct stat info;
    long long when = -1;
    if (stat(path, &info) == 0) {
#ifdef __APPLE__
        struct timespec at = info.st_mtimespec;
#else
        struct timespec at = info.st_mtim;
#endif
        when = (long long)at.tv_sec * 1000000000 + at.tv_nsec;
    }
    free(path);
    return when;
}

/* Keeps a long compiler or sanitizer report from flooding the screen. */
static void add_clipped(Str *s, const char *text) {
    Strings lines = split_lines(text);
    for (size_t i = 0; i < lines.count && i < MAX_DETAIL_LINES; i++) {
        str_addf(s, "%s\n", lines.items[i]);
    }
    if (lines.count > MAX_DETAIL_LINES) {
        str_addf(s, RESET DIM "… %zu more lines not shown" RESET "\n", lines.count - MAX_DETAIL_LINES);
    }
    strings_free(&lines);
}

static void add_repeated(Str *s, const char *text, size_t times) {
    for (size_t i = 0; i < times; i++) {
        str_add(s, text);
    }
}

static void render(const Session *s) {
    const Course *course = s->course;
    const Exercise *ex = current(s);
    size_t total = course->count;
    size_t done = state_done_count(&s->state, course);
    size_t filled = done * BAR_WIDTH / total;
    char *working = course_working(course, ex);
    char *solution = course_solution(course, ex);
    const char *separator = " " DIM "·" RESET " ";

    Str out = {0};
    str_addf(&out, BOLD "%s" RESET "%s%s%sexercise %zu of %zu%s%s\n", ex->name, separator, ex->dir, separator,
             s->index + 1, total, separator, kind_label(ex->kind));
    str_add(&out, GREEN);
    add_repeated(&out, "█", filled);
    str_add(&out, RESET DIM);
    add_repeated(&out, "░", BAR_WIDTH - filled);
    str_addf(&out, RESET " %zu/%zu done\n\n", done, total);
    str_addf(&out, "Open in your editor: " CYAN "%s" RESET "\n\n", course_relative(course, working));

    if (s->outcome.passed) {
        str_add(&out, GREEN BOLD "✓ Passed" RESET "\n\n");
        if (!is_blank(s->outcome.output)) {
            str_add(&out, "Output:\n");
            add_clipped(&out, s->outcome.output);
            str_add(&out, "\n");
        }
        str_addf(&out, "Reference solution: " CYAN "%s" RESET "\n", course_relative(course, solution));
    } else {
        str_addf(&out, RED BOLD "✗ %s" RESET "\n\n", s->outcome.title);
        add_clipped(&out, s->outcome.detail);
    }

    for (size_t i = 0; i < s->hints_shown && i < ex->hints.count; i++) {
        str_addf(&out, "\n" YELLOW "Hint %zu of %zu:" RESET " %s\n", i + 1, ex->hints.count, ex->hints.items[i]);
    }
    if (s->note != NULL) {
        str_addf(&out, "\n" YELLOW "%s" RESET "\n", s->note);
    }

    str_add(&out, "\n");
    if (s->outcome.passed) {
        str_addf(&out, BOLD "n" RESET " next%s", separator);
    }
    str_addf(&out, BOLD "b" RESET " back%s" BOLD "h" RESET " hint%s" BOLD "l" RESET " list%s" BOLD "r" RESET
                        " re-run%s" BOLD "x" RESET " reset%s" BOLD "R" RESET " restart course%s" BOLD "q" RESET
                        " quit\n",
             separator, separator, separator, separator, separator, separator);
    show(out.data);

    str_free(&out);
    free(working);
    free(solution);
}

static bool recheck(Session *s) {
    const Exercise *ex = current(s);
    char *note = format(DIM "Checking %s…" RESET "\n", ex->name);
    show(note);
    free(note);

    char *working = course_working(s->course, ex);
    outcome_free(&s->outcome);
    s->outcome = check(s->course, ex, working);
    free(working);
    if (s->outcome.passed) {
        state_mark_done(&s->state, ex);
    }
    return state_save(&s->state, s->course);
}

static bool go_to(Session *s, size_t index) {
    s->index = index;
    s->hints_shown = 0;
    state_set_current(&s->state, current(s));
    if (!recheck(s)) {
        return false;
    }
    render(s);
    return true;
}

static void hint(Session *s) {
    size_t total = current(s)->hints.count;
    if (total == 0) {
        s->note = "This exercise has no hints.";
    } else if (s->hints_shown == total) {
        s->note = "That was the last hint.";
    } else {
        s->hints_shown++;
    }
}

static bool next(Session *s) {
    size_t index;
    if (!s->outcome.passed) {
        s->note = "This exercise is not passing yet. Press l to jump to another one.";
        return true;
    }
    if (!state_next_pending(&s->state, s->course, s->index, &index)) {
        s->note = "You have finished every exercise in the course. Well done!";
        return true;
    }
    return go_to(s, index);
}

/* Shows `question` and waits for a key; true only if it was y. */
static bool confirm(Session *s, const char *question) {
    s->note = question;
    render(s);
    s->note = NULL;

    int key;
    do {
        key = read_key(-1);
    } while (key == KEY_NONE && !stopped);
    return key == 'y';
}

static bool back(Session *s) {
    if (s->index == 0) {
        s->note = "This is the first exercise.";
        return true;
    }
    return go_to(s, s->index - 1);
}

static bool reset(Session *s) {
    if (!confirm(s, "Reset this file to its original state? Your changes will be lost. Press y to confirm.")) {
        return true;
    }
    if (!course_reset(s->course, current(s))) {
        return false;
    }
    s->hints_shown = 0;
    if (!recheck(s)) {
        return false;
    }
    s->note = "File reset. Reload it in your editor.";
    return true;
}

/* Starts the whole course over: every file back to its original, no progress. */
static bool restart(Session *s) {
    if (!confirm(s, "Restart the whole course? Every exercise file is reset and all progress is erased. "
                    "Press y to confirm.")) {
        return true;
    }
    for (size_t i = 0; i < s->course->count; i++) {
        if (!course_reset(s->course, &s->course->exercises[i])) {
            return false;
        }
    }
    state_free(&s->state);
    if (!go_to(s, 0)) {
        return false;
    }
    s->note = "Course restarted. Reload the file in your editor.";
    return true;
}

/* The exercise list. Returns true, with the exercise to jump to, if one was chosen. */
static bool list(const Session *s, size_t *chosen) {
    const Course *course = s->course;
    size_t total = course->count;
    size_t cursor = s->index;
    while (!stopped) {
        size_t rows = terminal_rows() > 7 ? terminal_rows() - 4 : 3;
        size_t top = cursor > rows / 2 ? cursor - rows / 2 : 0;
        size_t last_top = total > rows ? total - rows : 0;
        if (top > last_top) {
            top = last_top;
        }

        Str out = {0};
        str_add(&out, BOLD "Exercises" RESET "\n\n");
        for (size_t i = top; i < total && i < top + rows; i++) {
            const Exercise *ex = &course->exercises[i];
            bool here = i == cursor;
            str_addf(&out, "%s %s %3zu  " DIM "%-16s" RESET " %s%s" RESET " " DIM "(%s)" RESET "\n",
                     here ? CYAN ">" RESET : " ", state_is_done(&s->state, ex) ? GREEN "✓" RESET : DIM "·" RESET,
                     i + 1, ex->dir, here ? BOLD : "", ex->name, kind_label(ex->kind));
        }
        str_add(&out, "\n" BOLD "↑ ↓" RESET " move " DIM "·" RESET " " BOLD "enter" RESET " open " DIM "·" RESET
                      " " BOLD "q" RESET " back\n");
        show(out.data);
        str_free(&out);

        /* A resize interrupts the wait, and the list is then drawn again. */
        int key = read_key(-1);
        resized = 0;
        if (key == KEY_UP || key == 'k') {
            cursor -= cursor > 0;
        } else if (key == KEY_DOWN || key == 'j') {
            cursor += cursor + 1 < total;
        } else if (key == KEY_ENTER) {
            *chosen = cursor;
            return true;
        } else if (key == KEY_ESC || key == 'l' || is_quit(key)) {
            return false;
        }
    }
    return false;
}

static bool watch(Session *s) {
    long long seen = modified(s);
    while (!stopped) {
        int key = read_key(POLL_MS);
        if (is_quit(key)) {
            return true;
        }
        if (key != KEY_NONE) {
            size_t chosen;
            bool ok = true;
            s->note = NULL;
            switch (key) {
            case 'h': hint(s); break;
            case 'n': ok = next(s); break;
            case 'b': ok = back(s); break;
            case 'r': ok = recheck(s); break;
            case 'x': ok = reset(s); break;
            case 'R': ok = restart(s); break;
            case 'l': ok = !list(s, &chosen) || go_to(s, chosen); break;
            default: break;
            }
            if (!ok) {
                return false;
            }
            render(s);
            seen = modified(s);
        } else if (resized) {
            resized = 0;
            render(s);
        }

        long long now = modified(s);
        if (now >= 0 && now != seen) {
            /* Give the editor a moment to finish writing. */
            sleep_ms(50);
            seen = modified(s);
            s->note = NULL;
            if (!recheck(s)) {
                return false;
            }
            render(s);
        }
    }
    return true;
}

bool watch_run(const Course *course) {
    if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO)) {
        fail("watch mode needs an interactive terminal");
        return false;
    }
    if (!course_ensure_working(course)) {
        return false;
    }

    Session session = {.course = course, .state = state_load(course)};
    /* Resume where the learner stopped, or else at the first unfinished exercise. */
    if (!course_index_of(course, session.state.current, &session.index) &&
        !state_next_pending(&session.state, course, course->count - 1, &session.index)) {
        session.index = 0;
    }

    bool ok = enter_raw_mode() && go_to(&session, session.index) && watch(&session);
    if (ok) {
        show("");
    }
    leave_raw_mode();
    outcome_free(&session.outcome);
    state_free(&session.state);
    return ok;
}
