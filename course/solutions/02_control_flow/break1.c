/* Even numbers are skipped; at 9 the loop is left for good. */

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
1
3
5
7
done
*/
