#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int test_readlink(int argc, char *argv[])
{
	char *path;
	char str[256];
	int r;

	memset(str, 0x0, sizeof(str));

	if (argc > 1)
		path = argv[1];
	else
		return -1;

	r = readlink(path, str, sizeof(str));
	if (r < 0) {
		perror("readlink error");
		return -1;
	}
	printf("link %s = %s\n", path, str);

	return 0;
}

int test_access(int argc, char *argv[])
{
	int r;
	char *file_dir;

	// get file directory
	if (argc > 1)
		file_dir = argv[1];
	else
		file_dir = "./a.out";

	// existence
	r = access(file_dir, F_OK);
	if (!r) {
		printf("file %s exists\n", file_dir);
	} else {
		printf("not find %s\n", file_dir);
		return 0;
	}

	// check readable
	r = access(file_dir, R_OK);
	if (!r)
		printf("file %s is readable\n", file_dir);
	else
		printf("file %s is not readable\n", file_dir);

	// check writable
	r = access(file_dir, W_OK);
	if (!r)
		printf("file %s is writable\n", file_dir);
	else
		printf("file %s is not writable\n", file_dir);

	// check executable
	r = access(file_dir, X_OK);
	if (!r)
		printf("file %s is executable\n", file_dir);
	else
		printf("file %s is not executable\n", file_dir);

	return 0;
}

int main(int argc, char *argv[])
{
//	test_readlink(argc, argv);
	test_access(argc, argv);
	return 0;
}
