/* Compiling an exercise, running it, and deciding whether it passes. */

#ifndef CCOURSE_CHECK_H
#define CCOURSE_CHECK_H

#include <stdbool.h>

#include "course.h"

typedef struct {
    bool passed;
    /* What the program printed, when it passed. */
    char *output;
    /* What went wrong, when it did not. */
    char *title;
    char *detail;
} Outcome;

/* Judges the exercise using the given source file: the learner's copy, the
 * original or the solution. */
Outcome check(const Course *course, const Exercise *ex, const char *source);
void outcome_free(Outcome *outcome);

#endif
