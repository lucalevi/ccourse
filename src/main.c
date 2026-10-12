/* ccourse: learn C in the terminal. */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "check.h"
#include "course.h"
#include "util.h"
#include "watch.h"

static const char HELP[] =
    "ccourse: learn C in the terminal\n"
    "\n"
    "Usage:\n"
    "  ccourse           start the course (watch mode)\n"
    "  ccourse verify    for course authors: check every exercise and solution\n"
    "  ccourse --help    show this text\n"
    "\n"
    "Run it from the course directory, keep it open, and edit the exercise file\n"
    "it names in your editor. Each time you save, the exercise is checked again.\n";

/* Prints text with every line after the first indented like the first. */
static void print_indented(const char *text) {
    Strings lines = split_lines(text);
    for (size_t i = 0; i < lines.count; i++) {
        printf("      %s\n", lines.items[i]);
    }
    strings_free(&lines);
}

/* Every original exercise must fail and every reference solution must pass. */
static bool verify(const Course *course) {
    size_t problems = 0;
    for (size_t i = 0; i < course->count; i++) {
        const Exercise *ex = &course->exercises[i];
        Str faults = {0};

        char *pristine = course_pristine(course, ex);
        Outcome outcome = check(course, ex, pristine);
        if (outcome.passed) {
            str_add(&faults, "the untouched exercise already passes\n");
        }
        outcome_free(&outcome);
        free(pristine);

        char *solution = course_solution(course, ex);
        outcome = check(course, ex, solution);
        if (!outcome.passed) {
            str_addf(&faults, "the solution fails: %s\n%s", outcome.title, outcome.detail);
        }
        outcome_free(&outcome);
        free(solution);

        if (ex->hints.count == 0) {
            str_add(&faults, "no hints\n");
        }

        if (faults.len == 0) {
            printf(GREEN "ok" RESET "    %s/%s\n", ex->dir, ex->name);
        } else {
            problems++;
            printf(RED "FAIL" RESET "  %s/%s\n", ex->dir, ex->name);
            print_indented(faults.data);
        }
        fflush(stdout);
        str_free(&faults);
    }
    if (problems > 0) {
        fail("%zu of %zu exercises have problems", problems, course->count);
        return false;
    }
    printf("\nAll %zu exercises verified.\n", course->count);
    return true;
}

int main(int argc, char **argv) {
    const char *command = argc > 1 ? argv[1] : NULL;
    bool ok;
    if (command != NULL && (strcmp(command, "-h") == 0 || strcmp(command, "--help") == 0)) {
        fputs(HELP, stdout);
        ok = true;
    } else if (command != NULL && strcmp(command, "verify") != 0) {
        fail("unknown argument '%s' (try --help)", command);
        ok = false;
    } else {
        /* An exercise may exit without reading the input we are sending it. */
        signal(SIGPIPE, SIG_IGN);
        Course course;
        ok = course_load(&course);
        if (ok) {
            ok = command == NULL ? watch_run(&course) : verify(&course);
            course_free(&course);
        }
    }
    if (!ok) {
        fprintf(stderr, "ccourse: %s\n", failure());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
