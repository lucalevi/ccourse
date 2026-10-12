/* Using double throughout avoids integer division. */

#include <stdio.h>

int main(void) {
    double low = 32.0;
    double body = 98.6;
    double boiling = 212.0;

    printf("%5.1f F = %5.1f C\n", low, (low - 32) * 5 / 9);
    printf("%5.1f F = %5.1f C\n", body, (body - 32) * 5 / 9);
    printf("%5.1f F = %5.1f C\n", boiling, (boiling - 32) * 5 / 9);
    return 0;
}
