/*
 * division1: integer division
 *
 * The arithmetic operators are + - * / and % (remainder).
 *
 * When both sides of / are int, the result is an int too: the fraction is
 * thrown away, not rounded. So 7 / 2 is 3. If at least one side is a
 * double, the division keeps the fraction: 7 / 2.0 is 3.5.
 *
 * The division happens first, with the types it sees. Storing the result
 * in a double afterwards does not bring the fraction back.
 *
 * This is a "debug" exercise: the program compiles cleanly, but its
 * result is wrong.
 *
 * TASK
 * Make the program print the true average:
 *
 *     average = 3.67
 */

#include <stdio.h>

int main(void) {
    int a = 3;
    int b = 4;
    int c = 4;

    double average = (a + b + c) / 3;

    printf("average = %.2f\n", average);
    return 0;
}
