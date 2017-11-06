#include <stdio.h>
#include <abc.h>

int func_add2(int a, int b)
{
#ifdef DEBUG
	printf("%s, a = %d, b = %d\n", __func__, a, b);
#endif
	return (a+b);
}
