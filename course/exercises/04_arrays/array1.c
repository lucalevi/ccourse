/*
 * array1: a row of values
 *
 * An array holds a fixed number of values of one type, side by side:
 *
 *     int primes[5] = {2, 3, 5, 7, 11};
 *
 * You reach one element with an index in square brackets. Counting starts
 * at zero, so the five elements are primes[0] to primes[4]. There is no
 * primes[5].
 *
 * TASK
 * Fix the two indexes so the program compiles without warnings and prints:
 *
 *     first: 2
 *     last: 11
 */

#include <stdio.h>

int main(void) {
    int primes[5] = {2, 3, 5, 7, 11};

    printf("first: %d\n", primes[1]);
    printf("last: %d\n", primes[5]);
    return 0;
}
