/*
 * const1: values that must not change
 *
 * Putting const in front of a declaration makes the variable read-only:
 *
 *     const int max_lives = 3;
 *
 * Any later attempt to assign to it is a compile error. Use const for
 * values that are fixed by design. The compiler then catches the mistake
 * of changing the wrong thing, which is exactly what happened here.
 *
 * TASK
 * The player loses one life. The maximum stays what it is. Make the
 * program compile and print:
 *
 *     lives = 2 of 3
 */

#include <stdio.h>

int main(void) {
    const int max_lives = 3;
    int lives = max_lives;

    max_lives = max_lives - 1;

    printf("lives = %d of %d\n", lives, max_lives);
    return 0;
}
