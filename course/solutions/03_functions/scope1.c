/* total is declared before the loop, so it outlives every round of it. */

#include <stdio.h>

int main(void) {
    int total = 0;

    for (int i = 1; i <= 4; i++) {
        total = total + i;
    }

    printf("total = %d\n", total);
    return 0;
}
