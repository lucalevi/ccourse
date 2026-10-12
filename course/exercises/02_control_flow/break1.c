/*
 * break1: leaving a loop early
 *
 *     break;       leaves the loop at once
 *     continue;    skips the rest of this round and goes on to the next
 *
 * TASK
 * Do not change the code. Write what it prints between the marker lines.
 */

#include <stdio.h>

int main(void) {
    for (int i = 1; i <= 10; i++) {
        if (i % 2 == 0) {
            continue;
        }
        if (i > 7) {
            break;
        }
        printf("%d\n", i);
    }
    printf("done\n");
    return 0;
}

/* PREDICT
*/
