/*
 * vars2: uninitialised variables
 *
 * Declaring a variable reserves memory for it but does not clean that
 * memory. Until you assign a value, the variable holds whatever happened
 * to be there before: garbage. C does not set it to zero for you.
 *
 * Reading such a variable is a bug, and one of the most common in C. This
 * is why the course compiles with warnings switched on: the compiler can
 * often spot it. A warning is not an error, so the program still builds,
 * but in this course (and in good practice) warnings are fixed, not ignored.
 *
 * TASK
 * Make the program compile without warnings and print:
 *
 *     count = 1
 */

#include <stdio.h>

int main(void) {
    int count;

    count = count + 1;
    printf("count = %d\n", count);
    return 0;
}
