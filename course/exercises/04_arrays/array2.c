/*
 * array2: looping over an array
 *
 * If you give an initialiser, you may leave the size out and the compiler
 * counts for you:
 *
 *     int values[] = {4, 8, 15};
 *
 * To get the count back, divide the size of the whole array in bytes by
 * the size of one element:
 *
 *     int count = sizeof(values) / sizeof(values[0]);
 *
 * This way the loop stays right when someone adds a value to the list.
 *
 * TASK
 * Compute count with sizeof, then add up all the values with a loop.
 * The program must print:
 *
 *     count = 6
 *     sum = 108
 */

#include <stdio.h>

int main(void) {
    int values[] = {4, 8, 15, 16, 23, 42};

    int count = 0; /* TODO: use sizeof */
    int sum = 0;

    /* TODO: add every element to sum */

    printf("count = %d\n", count);
    printf("sum = %d\n", sum);
    return 0;
}
