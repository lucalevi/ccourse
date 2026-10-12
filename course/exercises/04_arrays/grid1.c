/*
 * grid1: arrays of arrays
 *
 * A table is an array whose elements are arrays:
 *
 *     int grid[3][3] = {
 *         {1, 2, 3},
 *         {4, 5, 6},
 *         {7, 8, 9},
 *     };
 *
 * The first index picks the row, the second the column: grid[1][2] is 6.
 * Tables are usually walked with a loop inside a loop.
 *
 * TASK
 * Fill in the inner loop so the program prints the sum of each row:
 *
 *     row 0: 6
 *     row 1: 15
 *     row 2: 24
 */

#include <stdio.h>

int main(void) {
    int grid[3][3] = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9},
    };

    for (int row = 0; row < 3; row++) {
        int sum = 0;

        /* TODO: add the three numbers of this row to sum */

        printf("row %d: %d\n", row, sum);
    }
    return 0;
}
