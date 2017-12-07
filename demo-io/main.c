#include <stdio.h>
#include <string.h>
#include "comm_file.h"

int main(int argc, char *argv[])
{
	struct comm_file *file;
	char *path = "cf.txt";
	char *dat = "helloworld";
	char *app = "program";
	char buf[512];
	int r, len, rxlen;

	if (argc > 1)
		path = argv[1];

	file = comm_file_create("file1");

	// create a new file
	printf("create new file ...\n");
	r = file->open(file, path);
	if (r) {
		printf("open file failed\n");
		goto out;
	}

	r = file->write(file, dat, strlen(dat));
	printf("write %d bytes to file\n", r);

	r = file->close(file);
	if (r) {
		perror("close file failed\n");
		goto out;
	}

	// append string to file
	printf("append string to file ...\n");
	r = file->open(file, path);
	if (r) {
		printf("open file failed\n");
		goto out;
	}

	r = file->append(file, app, strlen(app));
	r += file->append(file, app, strlen(app));
	r += file->insert(file, -7, "xyz", 3);
	printf("write %d bytes to file\n", r);

	r = file->close(file);
	if (r) {
		perror("close file failed\n");
		goto out;
	}

	// read string from file
	printf("read string to file ...\n");
	memset(buf, 0x0, 100);

	r = file->open(file, path);
	if (r) {
		perror("open file failed\n");
		goto out;
	}

	len =file->at_end(file);
	printf("len = %d\n", len);

	file->at_start(file);
	rxlen = file->read(file, buf, len, 1);
	printf("read %d bytes\n", rxlen);

	r = file->close(file);
	if (r) {
		perror("close file failed\n");
		goto out;
	}

	printf("\nfile data:\n");
	printf("%s", buf);
	printf("\n");

out:
	comm_file_destroy(file);
	return 0;
}
