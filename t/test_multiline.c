/*
A=b;C="d" ; E = "F;" ;
*/
#include <stdio.h>
#include "../libHX.h"

int main(void)
{
    char *A, *C, *E;
    struct HXoption opt_tab[] = {
        {.ln = "A", .type = HXTYPE_STRING, .ptr = &A},
        {.ln = "C", .type = HXTYPE_STRING, .ptr = &C},
        {.ln = "E", .type = HXTYPE_STRING, .ptr = &E},
        HXOPT_TABLEEND,
    };
    HX_shconfig("test_multiline.c", opt_tab);
    printf("A => >%s<\n" "C => >%s<\n" "E => >%s<\n", A, C, E);
    return 0;
}
