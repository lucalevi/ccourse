/*
 * bounds1: C does not check your indexes
 *
 * If you use an index outside the array, C does not stop you. The program
 * reads or overwrites whatever lies next to the array in memory. It may
 * appear to work, print rubbish, or crash, today or next month. The C
 * standard calls this "undefined behaviour": anything may happen.
 *
 * Because such bugs can hide, from now on the course builds your programs
 * with "sanitizers". They add checks to the program that C leaves out,
 * and stop it with a report the moment it touches memory it should not.
 *
 * Reading a sanitizer report:
 *   - a line containing "runtime error" says what went wrong, and starts
 *     with the file, line and column where it happened;
 *   - the AddressSanitizer part names the kind of error (here a
 *     stack-buffer-overflow), and its lines #0, #1... show where the
 *     program was at that moment;
 *   - further down it tells you which variable was overrun.
 *
 * TASK
 * This program compiles without a single warning, yet it is wrong. Find
 * the line the report points to and fix both loops so the program prints
 * the squares of 0 to 4:
 *
 *     0
 *     1
 *     4
 *     9
 *     16
 */

#include <stdio.h>

int main(void) {
    int squares[5];

    for (int i = 0; i <= 5; i++) {
        squares[i] = i * i;
    }
    for (int i = 0; i <= 5; i++) {
        printf("%d\n", squares[i]);
    }
    return 0;
}
