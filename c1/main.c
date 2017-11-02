#include <stdio.h>
#include <abc.h>

int main(int argc, const char *argv[])
{
	int n, a = 1, b = 1;

	printf("argc=%d\n", argc);
	if(argc > 0) {
		const char *fname = argv[0];
		printf("file name: %s\n", fname);
	}
	if (argc > 1)
		a = *(int *)(argv[1]);
	if (argc > 2)
		b = *(int *)(argv[2]);

	n = func_add2(a,b);
	printf("n = %d\n", n);

	return 0;
}

