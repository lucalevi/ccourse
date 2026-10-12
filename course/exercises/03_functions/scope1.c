/*
 * scope1: where a variable lives
 *
 * A variable exists only inside the braces { } where it is declared.
 * That region is its scope. When execution leaves the braces, the
 * variable is gone, and the name means nothing outside them.
 *
 * A loop body is such a pair of braces, and it is entered afresh on every
 * round. A variable declared inside it is created anew each time.
 *
 * TASK
 * This program should add up 1 + 2 + 3 + 4. Make it compile and print:
 *
 *     total = 10
 */

#include <stdio.h>

int main(void) {
    for (int i = 1; i <= 4; i++) {
        int total = 0;
        total = total + i;
    }

    printf("total = %d\n", total);
    return 0;
}
