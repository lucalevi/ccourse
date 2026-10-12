/* The course on disk: the exercise list, where files live, and saved progress. */

#ifndef CCOURSE_COURSE_H
#define CCOURSE_COURSE_H

#include <stdbool.h>
#include <stddef.h>

#include "util.h"

typedef enum { KIND_FIX, KIND_COMPLETE, KIND_PREDICT, KIND_DEBUG, KIND_BUILD } Kind;

const char *kind_label(Kind kind);

typedef struct {
    Strings args;
    /* Text given to the program on standard input; NULL means none. */
    char *input;
    /* Expected standard output; not checked when NULL. */
    char *output;
    int exit_code;
} Test;

typedef struct {
    char *name;
    char *dir;
    Kind kind;
    /* Build with AddressSanitizer and UndefinedBehaviorSanitizer. */
    bool sanitize;
    Strings hints;
    Test *tests;
    size_t test_count;
} Exercise;

typedef struct {
    char *root;
    Exercise *exercises;
    size_t count;
} Course;

/* Finds `course/info.toml` in the current directory or any parent. */
bool course_load(Course *course);
void course_free(Course *course);

/* Where the files of an exercise are. The caller frees each path. */
char *course_pristine(const Course *course, const Exercise *ex); /* the untouched original */
char *course_solution(const Course *course, const Exercise *ex);
char *course_working(const Course *course, const Exercise *ex); /* the learner's copy, the one watched */
char *course_bin(const Course *course, const Exercise *ex);     /* the compiled program */

/* Shortens a path for display and for compiler messages. */
const char *course_relative(const Course *course, const char *path);

/* Gives the learner a copy of every exercise they do not have yet. */
bool course_ensure_working(const Course *course);
/* Replaces the learner's copy with the original. */
bool course_reset(const Course *course, const Exercise *ex);

/* Finds an exercise by name. */
bool course_index_of(const Course *course, const char *name, size_t *index);

typedef struct {
    char *current;
    Strings done;
} State;

/* Progress saved by an earlier run, or a fresh start if there is none. */
State state_load(const Course *course);
bool state_save(const State *state, const Course *course);
void state_free(State *state);

bool state_is_done(const State *state, const Exercise *ex);
void state_mark_done(State *state, const Exercise *ex);
void state_set_current(State *state, const Exercise *ex);
size_t state_done_count(const State *state, const Course *course);

/* The first unfinished exercise, searching from `from` and wrapping round to
 * the start. `from` itself is considered last. */
bool state_next_pending(const State *state, const Course *course, size_t from, size_t *index);

#endif
