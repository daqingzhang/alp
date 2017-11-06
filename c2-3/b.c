#include <stdio.h>
#include <abc.h>

int func_add3(int a, int b, int c)
{
#ifdef DEBUG
	printf("a = %d, b = %d, c = %d\n", a, b, c);
#endif
	return (a+b+c);
}
