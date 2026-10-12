/*
 * recursion1: a function that calls itself
 *
 * The factorial of n is n * (n-1) * (n-2) * ... * 1, and the factorial
 * of 0 is defined as 1. Put differently:
 *
 *     factorial(n) = n * factorial(n - 1)
 *
 * A function may call itself, so that line can be written almost as it
 * stands. But each call waits for the one it made, and every waiting call
 * occupies memory on "the stack". There must be a case that answers
 * directly, without another call. That is the base case. Without one,
 * the calls pile up until the stack is full and the program crashes.
 *
 * TASK
 * Add the base case so the program prints:
 *
 *     1
 *     120
 *     3628800
 */

#include <stdio.h>

int factorial(int n) {
    /* TODO: the base case */
    return n * factorial(n - 1);
}

int main(void) {
    printf("%d\n", factorial(0));
    printf("%d\n", factorial(5));
    printf("%d\n", factorial(10));
    return 0;
}
