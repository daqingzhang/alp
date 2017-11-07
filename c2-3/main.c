#include <stdio.h>
#include <abc.h>

int func_inc(int a)
{
#ifdef DEBUG
	printf("%s, a=%d\n", __func__, a);
#endif
	return (a+1);
}

int main(int argc, char *argv[])
{
	int s;

	s = func_add2(1,2);
	printf("s = %d\n", s);

	s = func_add3(1,2,3);
	printf("s = %d\n", s);

	s = func_inc(s);
	printf("s = %d\n", s);

	return 0;
}
