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

#include <curses.h>
enum arrow_key{
	AK_UP,
	AK_DOWN,
	AK_RIGHT,
	AK_LEFT,

	AK_QTY,
};

#define AK_SIZE 3
char arrow_key_val[AK_QTY][3] = {
	{0x1b, 0x5b, 0x41},
	{0x1b, 0x5b, 0x42},
	{0x1b, 0x5b, 0x43},
	{0x1b, 0x5b, 0x44},
};

int akey_match(char *buf, int cnt)
{
	int i, j, k = -1;
	char *key;

	if (cnt < AK_SIZE) {
		return k;
	}
	cnt /= AK_SIZE;

	for(i = 0; i < AK_QTY; i++) {
		key = &arrow_key_val[i][0];
		for(j = 0; j < cnt; j += AK_SIZE) {
			if ((buf[j] == key[0])
				&& (buf[j+1] == key[1])
				&& (buf[j+2] == key[2])) {
					k = i;
					break;
			}
		}
	}
	return k;
}

void test1(void)
{
	char ch, buf[100];
	int i = 0, j = 0;
	int k;

	printf("CMD>\n");
	initscr();
	do {
		ch = getch();
		buf[j++] = ch;
		k =akey_match(buf, j);

		if (ch == 0xa) {
			break;
		}
	} while(1);
	endwin();

	for(i = 0; i < j; i++) {
		printf("buf[%d] = %x\n", i, buf[i]);
	}
	printf("k=%d\n", k);
}

int test_main(int argc, char *argv[])
{
	printf("%s\n", __func__);

//	test_libusb(argc, argv);
//	usb_cmd_test_handler(argc, argv);
	test1();
	return 0;
}

