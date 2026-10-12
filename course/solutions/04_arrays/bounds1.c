/* i <= 5 let i reach 5, one past the last valid index. i < 5 is right. */

#include <stdio.h>

int main(void) {
    int squares[5];

    for (int i = 0; i < 5; i++) {
        squares[i] = i * i;
    }
    for (int i = 0; i < 5; i++) {
        printf("%d\n", squares[i]);
    }
    return 0;
}
