/* The loop runs from index 0 up to, but not including, count. */

#include <stdio.h>

int main(void) {
    int values[] = {4, 8, 15, 16, 23, 42};

    int count = sizeof(values) / sizeof(values[0]);
    int sum = 0;

    for (int i = 0; i < count; i++) {
        sum = sum + values[i];
    }

    printf("count = %d\n", count);
    printf("sum = %d\n", sum);
    return 0;
}
