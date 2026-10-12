/* Watch mode: re-check the current exercise whenever its file is saved. */

#ifndef CCOURSE_WATCH_H
#define CCOURSE_WATCH_H

#include <stdbool.h>

#include "course.h"

/* Runs until the learner quits. */
bool watch_run(const Course *course);

#endif
