/*
	Fails to compile with gcc-4.7, 4.8 with the error message
	"const_cast.c:5:13: error: dereferencing pointer to incomplete type".
	But __typeof__(*f) is just a fancy way of writing "struct undisclosed"
	and should be permitted. (Request for enhancement)
*/
struct undisclosed;
int main(void) {
	const struct undisclosed *f = 0;
	__typeof__(*f) *g = 0;
	return 0;
}
