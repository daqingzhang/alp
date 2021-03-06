#include <stdio.h>
#include <string.h>

#define DBG printf

static unsigned int astrtol(const char *nptr, char **endptr, int base)
{
	const char *s = nptr;
	unsigned int v = 0, t;
	int i, pos, len;

	DBG("%s, s:%s, base=%d\n", __func__, nptr, base);

	if (base == 0) {
		if (s[0] == '0') {
			if ((s[1] == 'x') || (s[1] == 'X'))
				base = 16;
			else
				base = 8;
		} else {
			base = 10;
		}
	}
	if (base == 10) {
		pos = 1;
		len = strlen(s);
		for (i = len - 1;i >= 0; i--) {
			if ((s[i] >= '0') && (s[i] <= '9'))
				t = s[i] - 0x30;
			else
				break;
			pos *= base;
			v += (t * (pos / base));
		}
	} else if (base == 8) {
		if (s[0] == '0')
			s++;
		pos = 1;
		len = strlen(s);
		for (i = len - 1;i >= 0; i--) {
			if ((s[i] >= '0') && (s[i] <= '7'))
				t = s[i] - 0x30;
			else
				break;
			pos *= base;
			v += (t * (pos / base));
		}
	} else {
		if (((s[0] == '0') && (s[1] == 'x'))
			|| ((s[0] == '0') && (s[1] == 'X')))
			s += 2;
		len = strlen(s);
		for (i = 0; i < len; i++) {
			if ((*s >= 0x30) && (*s <= 0x39))
				t = *s - 0x30;
			else if ((*s >= 'a') && (*s <= 'f'))
				t = *s - 0x57;
			else if ((*s >= 'A') && (*s <= 'F'))
				t = *s - 0x37;
			else
				break;
			pos = ((len - 1) - i) << 2;
			v |= (t << pos);
			s++;
		}
	}
	DBG("%s, v=0x%x\n", __func__, v);
	return v;
}

int main(int argc, char *argv[])
{
#if 0
	char hex1[] = "0x1234abcd";
	char hex2[] = "0X12de";
	char dec1[] = "12345678";
	char dec2[] = "1234";
	char oct1[] = "012";
	char oct2[] = "23";

	unsigned int tmp;

	tmp = astrtol(hex1, 0, 16);
	printf("tmp=%x\n", tmp);

	tmp = astrtol(hex2, 0, 16);
	printf("tmp=%x\n", tmp);

	tmp = astrtol(dec1, 0, 10);
	printf("tmp=%x\n", tmp);

	tmp = astrtol(dec2, 0, 10);
	printf("tmp=%x\n", tmp);

	tmp = astrtol(oct1, 0, 8);
	printf("tmp=%x\n", tmp);

	tmp = astrtol(oct2, 0, 8);
	printf("tmp=%x\n", tmp);

	tmp = astrtol(hex1, 0, 0);
	printf("tmp=%x\n", tmp);

	tmp = astrtol(dec1, 0, 0);
	printf("tmp=%x\n", tmp);

	tmp = astrtol(oct1, 0, 0);
	printf("tmp=%x\n", tmp);
#else
	unsigned int tmp;
	int i;

	for(i = 1; i < argc; i++) {
		tmp = astrtol(argv[i], 0, 0);
		printf("tmp=%x\n", tmp);
	}
#endif
	return 0;
}
