/*
 * value1: arguments are copied
 *
 * When you call a function, each argument's value is copied into the
 * matching parameter. The parameter is a new variable that belongs to the
 * function. Changing it changes the copy and nothing else.
 *
 * This is called "pass by value" and it is the only way C passes
 * arguments. It will matter a great deal when we reach pointers.
 *
 * TASK
 * Do not change the code. Write what it prints between the marker lines.
 */

#include <stdio.h>

void add_ten(int n) {
    n = n + 10;
    printf("inside: %d\n", n);
}

int main(void) {
    int x = 5;

    add_ten(x);
    printf("outside: %d\n", x);

    x = x + 1;
    add_ten(x);
    printf("outside: %d\n", x);
    return 0;
}

/* PREDICT
*/
