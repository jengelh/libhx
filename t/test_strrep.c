#include "../libHX.h"

int main(void)
{
    long uid = 1337;
    const char *str = "somestring";
    const char *pattern = "I like %k with uid %u";
    struct HXoption replace_catalog[] = {
        {.sh = 'u', .type = HXTYPE_LONG, .ptr = &uid},
        {.sh = 'k', .type = HXTYPE_STRING, .ptr = &str},
        HXOPT_TABLEEND,
    };
    printf(">%s<\n", HX_strrep(pattern, replace_catalog));
    return 0;
}
