/* square must return its result; print_line must not return a value. */

#include <stdio.h>

int square(int n) {
    return n * n;
}

void print_line(void) {
    printf("----\n");
}

int main(void) {
    print_line();
    printf("%d\n", square(7));
    print_line();
    return 0;
}
