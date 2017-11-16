#include <stdio.h>
#include <unistd.h>
#include <serial_thread.h>

char src[8] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37};
char dst[8] = {0};

int main(int argc, char *argv[])
{
	int r, i, cnt = 10;
	char *name;
	struct serial_obj *ser;

	if (argc < 2) {
		printf("port name ?\n");
		return -1;
	}
	name = argv[1];

	ser = serial_create("serial_obj");

	r = ser->open(ser, name);
	if (r) {
		printf("open serial failed, %d\n", r);
		return r;
	}

	while(cnt--) {
		r = ser->write(ser, src, 8);
		printf("%s, serial write %d bytes\n", __func__, r);
		sleep(1);

		r = ser->read(ser, dst, 8, 1);
		printf("%s, serial read %d bytes\n", __func__, r);
		for(i = 0;i < r; i++) {
			printf("dst[%d] = %x\n", i, dst[i]);
		}
		sleep(1);
	}
	ser->close(ser);
	serial_destroy(ser);
	return 0;
}
