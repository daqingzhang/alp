#include <stdio.h>
#include <abc.h>

int func_add3(int a, int b, int c)
{
#ifdef DEBUG
	printf("%s, a = %d, b = %d, c = %d\n", __func__, a, b, c);
#endif
	return (a+b+c);
}
