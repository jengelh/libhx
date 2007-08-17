#include <stdio.h>
#include <libHX.h>

static void cbf(const struct HXoptcb *cbi)
{
	printf("%s was called... with \"%s\"/'%c'\n", __func__,
	       cbi->current->ln, cbi->current->sh);
}

static int v = 0, mask = 0;
static char *kstr = NULL;
static long klong = 0;
static double kdbl = 0;
static int kflag = 0, kint = 0;
static int dst = 0;

static const char *eitheror[] = {"neither", "either", "or"};
static struct HXoption table[] = {
    {.ln = "dbl", .type = HXTYPE_DOUBLE, .cb = cbf, .ptr = &kdbl,
     .help = "Callback function for doubles"},
    {.ln = "flag", .sh = 'F', .type = HXTYPE_NONE, .cb = cbf, .ptr = &kflag,
     .help = "Callback function for flags"},
    {.ln = "long", .sh = 'L', .type = HXTYPE_LONG, .cb = cbf, .ptr = &klong,
     .help = "Callback function for integers"},
    {.ln = "str", .sh = 'S', .type = HXTYPE_STRING, .cb = cbf, .ptr = &kstr,
     .help = "Callback function for strings"},
    {.ln = "either", .type = HXTYPE_VAL, .ptr = &dst, .val = 1, .cb = cbf,
     .help = "Mutually exclusive selection: either | or"},
    {.ln = "or", .type = HXTYPE_VAL, .ptr = &dst, .val = 2, .cb = cbf,
     .help = "Mutually exclusive selection: either | or"},
    {.ln = "quiet", .sh = 'q', .type = HXOPT_DEC, .ptr = &v, .cb = cbf,
     .help = "Decrease verbosity"},
    {.ln = "verbose", .sh = 'v', .type = HXOPT_INC, .ptr = &v, .cb = cbf,
     .help = "Increase verbosity"},
    {.sh = 'A', .type = HXTYPE_INT | HXOPT_AND, .ptr = &mask, .cb = cbf,
     .help = "AND mask test", .htyp = "value"},
    {.sh = 'O', .type = HXTYPE_INT | HXOPT_OR, .ptr = &mask, .cb = cbf,
     .help = "OR mask test", .htyp = "value"},
    {.sh = 'X', .type = HXTYPE_INT | HXOPT_XOR, .ptr = &mask, .cb = cbf,
     .help = "XOR mask test", .htyp = "value"},
    {.sh = 'G', .type = HXTYPE_NONE, .help = "Just a flag", .cb = cbf},
    {.sh = 'H', .type = HXTYPE_NONE, .help = "Just a flag", .cb = cbf},
    {.sh = 'I', .type = HXTYPE_NONE, .help = "Just a flag", .cb = cbf},
    {.sh = 'J', .type = HXTYPE_NONE, .help = "Just a flag", .cb = cbf},
    HXOPT_AUTOHELP,
    HXOPT_TABLEEND,
};

int main(int argc, const char **argv)
{
	printf("Return value of HX_getopt: %d\n",
	       HX_getopt(table, &argc, &argv, HXOPT_USAGEONERR));
	printf("Either-or is: %s\n", eitheror[dst]);
	printf("values: D=%lf I=%d L=%ld S=%s\n", kdbl, kint, klong, kstr);
	printf("Verbosity level: %d\n", v);
	printf("Mask: 0x%08X\n", mask);
	return 0;
}
