#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

static unsigned char buf[1024 * 1024];

int read_test(int argc, char *argv[])
{
	char *fn;
	int wr = 0;
	int fd;
	int len, n, i;

	printf("%s\n", __func__);

	if (argc < 2) {
		printf("please specify file name\n");
		return -1;
	}
	if (argc > 1);
		fn = argv[1];

	if (argc > 2)
		wr = strtol(argv[2], NULL, 10);

	/* open file */
	fd = open(fn, O_RDWR);
	if (fd < 0) {
		perror("open file error\n");
		return -1;
	}

	/* get file size */
	len = lseek(fd, 0, SEEK_END);
	printf("fn=%s, fd=%d, len=%d, wr=%d\n", fn, fd, len, wr);

	/* read file data to buffer */
	lseek(fd, 0, SEEK_SET);
	n = read(fd, buf, len);
	printf("read %d B\n", n);

	/* print file data */
	for(i = 0;i < n; i++) {
		printf("%02x ", buf[i]);
		if ((i+1) % 8 == 0)
			printf("\n");
		if ((i+1) % 4 == 0)
			printf(" ");
	}
	printf("\n");

	/* load memory address is the last 4B data */
	unsigned int lma = *(unsigned int *)&buf[n-4];
	printf("lma = %x\n", lma);

	/* modify data */
	if (wr) {
		unsigned int magic_val = 0xbe57ec1c;
		*(unsigned int *)&buf[0] = magic_val;

		lseek(fd, 0, SEEK_SET);
		n = write(fd, (const void *)buf, len);
		printf("write %d B\n", n);
	}

#if 1
	/* test dup function */
	int fd2 = dup(fd);
	unsigned char tmp[100];

	if (fd2 < 0) {
		perror("dup failed\n");
		goto _exit;
	}
	printf("fd2=%d\n", fd2);

	lseek(fd2, 0, SEEK_SET);
	n = read(fd2, tmp, 32);

	for(i = 0;i < n; i++) {
		printf("%02x ",tmp[i]);
		if ((i+1) % 8 == 0)
			printf("\n");
		if ((i+1) % 4 == 0)
			printf(" ");
	}
#endif

_exit:
	close(fd);

	return 0;
}

int main(int argc, char *argv[])
{
	read_test(argc, argv);
	return 0;
}
