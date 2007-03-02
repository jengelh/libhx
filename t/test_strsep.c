#include "../libHX.h"

int main(void)
{
    char blather[] = "jengelh:x:1500:100:Jan Engelhardt:/home/jengelh:/bin/bash";
    char *wp = blather, *ret;
    while((ret = HX_strsep2(&wp, ":")) != NULL)
        printf("%s\n", ret);
    return 0;
}
