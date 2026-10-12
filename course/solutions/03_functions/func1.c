/* The prototype tells the compiler about greet before main calls it. */

#include <stdio.h>

void greet(void);

int main(void) {
    greet();
    greet();
    return 0;
}

void greet(void) {
    printf("Hello from a function!\n");
}
