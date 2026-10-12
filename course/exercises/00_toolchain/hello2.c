/*
 * hello2: including a header
 *
 * C itself knows almost nothing. Even printing is done by a function,
 * printf, that lives in the standard library. Before you may call a library
 * function, the compiler has to be told that it exists. That is the job of
 * a line such as
 *
 *     #include <stdio.h>
 *
 * stdio.h ("standard input/output") is the header that declares printf.
 * Include lines go at the top of the file and take no semicolon.
 *
 * TASK
 * Make this program compile.
 */

int main(void) {
    printf("Headers matter.\n");
    return 0;
}
