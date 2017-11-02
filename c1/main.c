#include <stdio.h>
#include <abc.h>

int main(int argc, const char *argv[])
{
	int n, a = 1, b = 1;
	const char *p;

	printf("argc=%d\n", argc);
	if (argc > 0) {
		const char *fname = argv[0];
		printf("file name: %s\n", fname);
	}
	if (argc > 1) {
		p = argv[1];
		printf("argv[1]: %s\n", p);
	}

	n = func_add2(a,b);
	printf("n = %d\n", n);

	return 0;
}

