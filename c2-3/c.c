#include <stdio.h>
#include <abc.h>

int func_mul3(int a, int b, int c)
{
#ifdef DEBUG
	printf("a = %d, b = %d, c = %d\n", a, b, c);
#endif
	a = a * b;
	a = a * c;
	return a;
}
