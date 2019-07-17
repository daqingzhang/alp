#include <stdio.h>
#include <unistd.h>
#include <serial_thread.h>
#include <usb_test.h>

#if 0
char src[8] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37};
char dst[8] = {0};

int test_main(int argc, char *argv[])
{
	int r, i, cnt = 10;
	char *name;
	struct serial_dev *ser;

	if (argc < 2) {
		printf("port name ?\n");
		return -1;
	}
	name = argv[1];

	ser = serial_dev_create("serial_obj");

	r = ser->open(ser, name);
	if (r) {
		printf("open serial failed, %d\n", r);
		return r;
	}

	ser->config(ser, 115200, 8, 'N', 1);

	while(cnt--) {
		r = ser->write(ser, src, 8);
		printf("%s, serial write %d bytes\n", __func__, r);
//		sleep(1);

		r = ser->read(ser, dst, 8, 1);
		printf("%s, serial read %d bytes\n", __func__, r);
		for(i = 0;i < r; i++) {
			printf("dst[%d] = %x\n", i, dst[i]);
		}
		sleep(1);
	}

	ser->close(ser);

	serial_dev_destroy(ser);

	return 0;
}
#endif

int usb_cmd_test_handler(int argc, char *argv[]);

int test_main(int argc, char *argv[])
{
	printf("%s\n", __func__);

//	test_libusb(argc, argv);
	usb_cmd_test_handler(argc, argv);
	return 0;
}

