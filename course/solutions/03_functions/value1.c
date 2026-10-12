/* add_ten changes its own copy, n. The caller's x is never touched by it. */

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
inside: 15
outside: 5
inside: 16
outside: 6
*/
