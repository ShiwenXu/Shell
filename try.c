#include <stdio.h>
#include <string.h>


int main()
{
    char *a;
    a = strdup("/");
    printf("%s\n", a);

    return 0;
}
