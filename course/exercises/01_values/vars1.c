/*
 * vars1: declaring variables
 *
 * A variable is a named place to keep a value. In C every variable has a
 * type, fixed for its whole life, and must be declared before it is used:
 *
 *     int apples = 4;      <- type, name, starting value
 *
 * int holds whole numbers. To print one, put %d in the printf text where
 * the number should appear, and pass the value after the text.
 *
 * TASK
 * Make this program compile and print:
 *
 *     apples: 4
 *     pears: 6
 */

#include <stdio.h>

int main(void) {
    int apples = 4;
    pears = 6;

    printf("apples: %d\n", apples);
    printf("pears: %d\n", pears);
    return 0;
}
