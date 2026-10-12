/* The loop variable i exists only inside the loop. */

#include <stdio.h>

int main(void) {
    for (int i = 1; i <= 5; i++) {
        printf("%d\n", i * i);
    }
    return 0;
}
