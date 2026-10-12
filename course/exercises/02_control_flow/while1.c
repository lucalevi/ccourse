/*
 * while1: repeating with while
 *
 *     while (condition) {
 *         ...body...
 *     }
 *
 * The condition is tested, and if it is true the body runs. Then the
 * condition is tested again, and so on. Something inside the body has to
 * change the condition eventually, or the loop never ends.
 *
 * Handy shorthand:  i++  means  i = i + 1.
 *
 * TASK
 * This program should add up the numbers 1 to 10, but it never finishes.
 * ccourse stops it after a few seconds. Fix it so it prints:
 *
 *     sum = 55
 */

#include <stdio.h>

int main(void) {
    int i = 1;
    int sum = 0;

    while (i <= 10) {
        sum = sum + i;
    }

    printf("sum = %d\n", sum);
    return 0;
}
