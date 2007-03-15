#include <stdio.h>
#include <libHX.h>

int main(void)
{
	char wall_1[4] = "ABCD", data[4] = "DATA", wall_2[4] = "XYZW";

	if(snprintf(data, sizeof(data), "12345678") >= sizeof(data))
		printf("Not enoguh space\n");
	printf("String: >%s<\n", data);

	HX_strlcat(data, "pqrstuv__", 2);
	printf("String: >%s<\n", data);

	*data = '\0';
	HX_strlcat(data, "123456789", sizeof(data));
	printf("String: >%s<\n", data);

	*data = '\0';
	HX_strlncat(data, "123456789", sizeof(data), 9);
	printf("String: >%s<\n", data);
	return 0;
}
