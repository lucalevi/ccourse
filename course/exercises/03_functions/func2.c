/*
 * func2: parameters and return values
 *
 *     int add(int a, int b) {
 *         return a + b;
 *     }
 *
 * Reading the first line: the function is called add, it takes two int
 * values which it knows as a and b, and it gives back an int. The return
 * statement ends the function and hands its value to the caller, where
 * the call add(2, 3) then stands for the value 5.
 *
 * TASK
 * Write a function called max that takes two int values and returns the
 * larger of the two. The program must print 9, 12 and 5, one per line.
 */

#include <stdio.h>

/* TODO: write max here */

int main(void) {
    printf("%d\n", max(3, 9));
    printf("%d\n", max(12, -4));
    printf("%d\n", max(5, 5));
    return 0;
}
