#include "course.h"

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "toml.h"

#define INFO_FILE "course/info.toml"
#define STATE_FILE ".ccourse/state.toml"

static const char *const KIND_LABELS[] = {
    [KIND_FIX] = "fix",
    [KIND_COMPLETE] = "complete",
    [KIND_PREDICT] = "predict",
    [KIND_DEBUG] = "debug",
    [KIND_BUILD] = "build",
};
#define KIND_COUNT (sizeof KIND_LABELS / sizeof KIND_LABELS[0])

const char *kind_label(Kind kind) {
    return KIND_LABELS[kind];
}

/* ── Reading course/info.toml ─────────────────────────────────────────────── */

typedef struct {
    Course *course;
    /* What the keys being read belong to; both NULL before the first table. */
    Exercise *exercise;
    Test *test;
    bool has_kind;
} Loader;

/* Checks the exercise that has just been read in full. */
static bool finish_exercise(const Loader *loader) {
    const Exercise *ex = loader->exercise;
    if (ex == NULL) {
        return true;
    }
    if (ex->name == NULL) {
        fail("an exercise has no `name`");
        return false;
    }
    if (ex->dir == NULL || !loader->has_kind) {
        fail("exercise '%s' has no `%s`", ex->name, ex->dir == NULL ? "dir" : "kind");
        return false;
    }
    for (const Exercise *other = loader->course->exercises; other < ex; other++) {
        if (strcmp(other->name, ex->name) == 0) {
            fail("two exercises are named '%s'; names must be unique", ex->name);
            return false;
        }
    }
    return true;
}

static bool on_table(void *ctx, const char *name) {
    Loader *loader = ctx;
    Course *course = loader->course;
    if (strcmp(name, "exercises") == 0) {
        if (!finish_exercise(loader)) {
            return false;
        }
        course->exercises = xrealloc(course->exercises, (course->count + 1) * sizeof *course->exercises);
        loader->exercise = &course->exercises[course->count++];
        *loader->exercise = (Exercise){0};
        loader->test = NULL;
        loader->has_kind = false;
        return true;
    }
    if (strcmp(name, "exercises.tests") == 0 && loader->exercise != NULL) {
        Exercise *ex = loader->exercise;
        ex->tests = xrealloc(ex->tests, (ex->test_count + 1) * sizeof *ex->tests);
        loader->test = &ex->tests[ex->test_count++];
        *loader->test = (Test){0};
        return true;
    }
    fail("unexpected table [[%s]]; use [[exercises]], then [[exercises.tests]] inside it", name);
    return false;
}

static bool expect(const TomlValue *value, TomlType type, const char *key) {
    static const char *const NAMES[] = {
        [TOML_STRING] = "a string",
        [TOML_INT] = "a whole number",
        [TOML_BOOL] = "true or false",
        [TOML_STRINGS] = "a list of strings",
    };
    if (value->type != type) {
        fail("`%s` must be %s", key, NAMES[type]);
        return false;
    }
    return true;
}

static bool set_string(char **slot, const TomlValue *value, const char *key) {
    if (!expect(value, TOML_STRING, key)) {
        return false;
    }
    free(*slot);
    *slot = xstrdup(value->string);
    return true;
}

static bool set_strings(Strings *slot, const TomlValue *value, const char *key) {
    if (!expect(value, TOML_STRINGS, key)) {
        return false;
    }
    strings_free(slot);
    for (size_t i = 0; i < value->strings.count; i++) {
        strings_push(slot, value->strings.items[i]);
    }
    return true;
}

static bool on_test_value(Test *test, const char *key, const TomlValue *value) {
    if (strcmp(key, "args") == 0) {
        return set_strings(&test->args, value, key);
    }
    if (strcmp(key, "stdin") == 0) {
        return set_string(&test->input, value, key);
    }
    if (strcmp(key, "stdout") == 0) {
        return set_string(&test->output, value, key);
    }
    if (strcmp(key, "exit") == 0) {
        if (!expect(value, TOML_INT, key)) {
            return false;
        }
        if (value->integer < INT_MIN || value->integer > INT_MAX) {
            fail("`exit` is out of range");
            return false;
        }
        test->exit_code = (int)value->integer;
        return true;
    }
    fail("unknown key `%s` in a test (the known ones are args, stdin, stdout, exit)", key);
    return false;
}

static bool on_exercise_value(Loader *loader, const char *key, const TomlValue *value) {
    Exercise *ex = loader->exercise;
    if (strcmp(key, "name") == 0) {
        return set_string(&ex->name, value, key);
    }
    if (strcmp(key, "dir") == 0) {
        return set_string(&ex->dir, value, key);
    }
    if (strcmp(key, "hints") == 0) {
        return set_strings(&ex->hints, value, key);
    }
    if (strcmp(key, "sanitize") == 0) {
        if (!expect(value, TOML_BOOL, key)) {
            return false;
        }
        ex->sanitize = value->boolean;
        return true;
    }
    if (strcmp(key, "kind") == 0) {
        if (!expect(value, TOML_STRING, key)) {
            return false;
        }
        for (size_t kind = 0; kind < KIND_COUNT; kind++) {
            if (strcmp(value->string, KIND_LABELS[kind]) == 0) {
                ex->kind = (Kind)kind;
                loader->has_kind = true;
                return true;
            }
        }
        fail("unknown kind '%s' (the kinds are fix, complete, predict, debug, build)", value->string);
        return false;
    }
    fail("unknown key `%s` in an exercise (the known ones are name, dir, kind, sanitize, hints)", key);
    return false;
}

static bool on_value(void *ctx, const char *key, const TomlValue *value) {
    Loader *loader = ctx;
    if (loader->test != NULL) {
        return on_test_value(loader->test, key, value);
    }
    if (loader->exercise != NULL) {
        return on_exercise_value(loader, key, value);
    }
    fail("`%s` is outside any exercise; start one with [[exercises]]", key);
    return false;
}

/* The nearest directory, from the current one upwards, that holds a course. */
static char *find_root(void) {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof cwd) == NULL) {
        fail("cannot find the current directory: %s", strerror(errno));
        return NULL;
    }
    for (;;) {
        char *info = format("%s/" INFO_FILE, cwd);
        bool found = is_file(info);
        free(info);
        if (found) {
            return xstrdup(cwd);
        }
        char *slash = strrchr(cwd, '/');
        if (slash == NULL || cwd[1] == '\0') {
            fail("no course found here (expected " INFO_FILE " in this directory or a parent)");
            return NULL;
        }
        /* Go up one directory; the last step up leaves just "/". */
        slash[slash == cwd ? 1 : 0] = '\0';
    }
}

bool course_load(Course *course) {
    *course = (Course){0};
    course->root = find_root();
    if (course->root == NULL) {
        return false;
    }
    char *path = format("%s/" INFO_FILE, course->root);
    char *text = read_file(path);
    free(path);
    if (text == NULL) {
        course_free(course);
        return false;
    }

    static const TomlHandler handler = {on_table, on_value};
    Loader loader = {.course = course};
    bool ok = toml_parse(text, &handler, &loader);
    free(text);
    ok = ok && finish_exercise(&loader);
    if (!ok) {
        fail(INFO_FILE ": %s", failure());
    } else if (course->count == 0) {
        fail(INFO_FILE " lists no exercises");
        ok = false;
    }
    if (!ok) {
        course_free(course);
    }
    return ok;
}

void course_free(Course *course) {
    for (size_t i = 0; i < course->count; i++) {
        Exercise *ex = &course->exercises[i];
        for (size_t t = 0; t < ex->test_count; t++) {
            strings_free(&ex->tests[t].args);
            free(ex->tests[t].input);
            free(ex->tests[t].output);
        }
        free(ex->tests);
        strings_free(&ex->hints);
        free(ex->name);
        free(ex->dir);
    }
    free(course->exercises);
    free(course->root);
    *course = (Course){0};
}

/* ── Where files live ─────────────────────────────────────────────────────── */

char *course_pristine(const Course *course, const Exercise *ex) {
    return format("%s/course/exercises/%s/%s.c", course->root, ex->dir, ex->name);
}

char *course_solution(const Course *course, const Exercise *ex) {
    return format("%s/course/solutions/%s/%s.c", course->root, ex->dir, ex->name);
}

char *course_working(const Course *course, const Exercise *ex) {
    return format("%s/exercises/%s/%s.c", course->root, ex->dir, ex->name);
}

char *course_bin(const Course *course, const Exercise *ex) {
    return format("%s/.ccourse/bin/%s", course->root, ex->name);
}

const char *course_relative(const Course *course, const char *path) {
    size_t len = strlen(course->root);
    if (strncmp(path, course->root, len) != 0 || path[len] != '/') {
        return path;
    }
    return path + len + 1;
}

bool course_reset(const Course *course, const Exercise *ex) {
    char *from = course_pristine(course, ex);
    char *to = course_working(course, ex);
    bool ok = make_parent_dirs(to) && copy_file(from, to);
    free(from);
    free(to);
    return ok;
}

bool course_ensure_working(const Course *course) {
    for (size_t i = 0; i < course->count; i++) {
        char *working = course_working(course, &course->exercises[i]);
        bool exists = access(working, F_OK) == 0;
        free(working);
        if (!exists && !course_reset(course, &course->exercises[i])) {
            return false;
        }
    }
    return true;
}

bool course_index_of(const Course *course, const char *name, size_t *index) {
    for (size_t i = 0; i < course->count; i++) {
        if (strcmp(course->exercises[i].name, name) == 0) {
            *index = i;
            return true;
        }
    }
    return false;
}

/* ── Saved progress ───────────────────────────────────────────────────────── */

static bool on_state_value(void *ctx, const char *key, const TomlValue *value) {
    State *state = ctx;
    if (strcmp(key, "current") == 0) {
        return set_string(&state->current, value, key);
    }
    if (strcmp(key, "done") == 0) {
        return set_strings(&state->done, value, key);
    }
    return true;
}

State state_load(const Course *course) {
    static const TomlHandler handler = {NULL, on_state_value};
    State state = {0};
    char *path = format("%s/" STATE_FILE, course->root);
    char *text = is_file(path) ? read_file(path) : NULL;
    /* A state file that cannot be understood counts as no progress. */
    if (text != NULL && !toml_parse(text, &handler, &state)) {
        state_free(&state);
    }
    if (state.current == NULL) {
        state.current = xstrdup("");
    }
    free(text);
    free(path);
    return state;
}

bool state_save(const State *state, const Course *course) {
    Str text = {0};
    str_add(&text, "current = ");
    toml_add_string(&text, state->current);
    str_add(&text, "\ndone = [");
    for (size_t i = 0; i < state->done.count; i++) {
        str_add(&text, i > 0 ? ", " : "");
        toml_add_string(&text, state->done.items[i]);
    }
    str_add(&text, "]\n");

    char *path = format("%s/" STATE_FILE, course->root);
    bool ok = make_parent_dirs(path) && write_file(path, text.data);
    free(path);
    str_free(&text);
    return ok;
}

void state_free(State *state) {
    free(state->current);
    strings_free(&state->done);
    *state = (State){0};
}

bool state_is_done(const State *state, const Exercise *ex) {
    return strings_contains(&state->done, ex->name);
}

void state_mark_done(State *state, const Exercise *ex) {
    if (!state_is_done(state, ex)) {
        strings_push(&state->done, ex->name);
    }
}

void state_set_current(State *state, const Exercise *ex) {
    free(state->current);
    state->current = xstrdup(ex->name);
}

size_t state_done_count(const State *state, const Course *course) {
    size_t done = 0;
    for (size_t i = 0; i < course->count; i++) {
        done += state_is_done(state, &course->exercises[i]);
    }
    return done;
}

bool state_next_pending(const State *state, const Course *course, size_t from, size_t *index) {
    for (size_t step = 1; step <= course->count; step++) {
        size_t i = (from + step) % course->count;
        if (!state_is_done(state, &course->exercises[i])) {
            *index = i;
            return true;
        }
    }
    return false;
}
