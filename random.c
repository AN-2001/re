#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main()
{
    int i;

    srand(time(NULL));
    for (i = 0; i < 2048 * 1024; i++)
        printf("%ld", random() % 2);

    return 0;
}
