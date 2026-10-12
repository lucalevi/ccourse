/* Without i++ the condition i <= 10 stayed true for ever. */

#include <stdio.h>

int main(void) {
    int i = 1;
    int sum = 0;

    while (i <= 10) {
        sum = sum + i;
        i++;
    }

    printf("sum = %d\n", sum);
    return 0;
}
