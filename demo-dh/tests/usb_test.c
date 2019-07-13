#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>
#include <usb_test.h>

#if 1
int add(int a, int c)
{
	return a+c;
}

void print_config_desc(struct libusb_config_descriptor *desc)
{
	if (!desc) {
		return;
	}
	printf("config desc info: \n");
	printf("bmAttrbutes: %d\n", desc->bmAttributes);
	printf("MaxPower:    %d\n", desc->MaxPower);
}

struct usb_message {
	struct libusb_control_setup b;
	uint8_t  *data;
	uint16_t length;
};

enum USB_OPCODE {
	OP_NULL,
	OP_RW_ANA,
	OP_RW_PMU,
	OP_RW_DIG,
	OP_RW_MEM,
	OP_EXEC,
	OP_RESET,
	OP_RESET_CDC,
};

#define REQ_TYPE_WR (LIBUSB_ENDPOINT_OUT \
		   | LIBUSB_REQUEST_TYPE_VENDOR \
		   | LIBUSB_RECIPIENT_OTHER)

#define REQ_TYPE_RD (LIBUSB_ENDPOINT_IN \
		   | LIBUSB_REQUEST_TYPE_VENDOR \
		   | LIBUSB_RECIPIENT_OTHER)

#if 0
#define USBLOG       printf
#else
#define USBLOG(...)  do{}while(0)
#endif

int usb_raw_write(libusb_device_handle *h, uint8_t opcode,
		uint32_t addr, uint8_t *data, uint32_t len)
{
	int r;
	struct usb_message msg;

	USBLOG("%s, opcode=%x, addr=%x, len=%d\n", __func__,
		opcode, addr, len);

	for(int i = 0; i < len; i++) {
		USBLOG("write data[%d]=%2x\n", i, data[i]);
	}

	msg.b.bmRequestType = REQ_TYPE_WR;
	msg.b.bRequest      = opcode;
	msg.b.wValue        = addr & 0xFFFF;
	msg.b.wIndex        = addr >> 16;
	msg.b.wLength       = (uint16_t)len;
	msg.data            = data;

	r = libusb_control_transfer(h, msg.b.bmRequestType,
		msg.b.bRequest, msg.b.wValue, msg.b.wIndex,
		msg.data, msg.b.wLength, 1000);

	if (r < 0) {
		perror("write usb dev");
	}
	USBLOG("r=%d\n", r);
	return r;
}

int usb_raw_read(libusb_device_handle *h, uint8_t opcode,
		uint32_t addr, uint8_t *data, uint32_t len)
{
	int r;
	struct usb_message msg;

	USBLOG("%s, opcode=%x, addr=%x, len=%d\n", __func__,
		opcode, addr, len);

	msg.b.bmRequestType = REQ_TYPE_RD;
	msg.b.bRequest      = opcode;
	msg.b.wValue        = addr & 0xFFFF;
	msg.b.wIndex        = addr >> 16;
	msg.b.wLength       = (uint16_t)len;
	msg.data            = data;

	r = libusb_control_transfer(h, msg.b.bmRequestType,
		msg.b.bRequest, msg.b.wValue, msg.b.wIndex,
		msg.data, msg.b.wLength, 1000);

	if (r < 0) {
		perror("read usb dev");
	}
	for(int i = 0; i < r; i++) {
		USBLOG("get data[%d]=%2x\n", i, data[i]);
	}
	USBLOG("r=%d\n", r);
	return r;
}

int test_device(uint16_t vid, uint16_t pid)
{
	libusb_device_handle *handle = NULL;
	//libusb_device *dev;
	//struct libusb_device_descriptor dev_desc;
	int r;

	r = libusb_init(NULL);
	if (r) {
		return -1;
	}

	handle = libusb_open_device_with_vid_pid(NULL, vid, pid);
	if (!handle) {
		perror("open libusb");
		return -1;
	}
	printf("open usb dev okay, vid=%x, pid=%x\n", vid, pid);

	uint32_t addr = 0x20000004;
	uint32_t data = 0x12345678;
	uint32_t recv_buf[32];
	//int rxlen = 32;
	//int i;

	//MEM test
	addr = 0x20000000;
	recv_buf[0] = 0;
	usb_raw_read(handle, OP_RW_MEM, addr, (uint8_t *)recv_buf,4);
	printf("mem 1[%x] = %x\n", addr, recv_buf[0]);

	usb_raw_write(handle, OP_RW_MEM, addr, (uint8_t *)&data, 4);
	recv_buf[0] = 0;
	usb_raw_read(handle, OP_RW_MEM, addr, (uint8_t *)recv_buf,4);
	printf("mem 2[%x] = %x\n", addr, recv_buf[0]);

	recv_buf[0] = 0;
	usb_raw_read(handle, OP_RW_MEM, addr, (uint8_t *)recv_buf,8);
	printf("mem 3[%x] = %x\n", addr, recv_buf[0]);
	printf("mem 4[%x] = %x\n", addr+4, recv_buf[1]);

	//ANA test
	addr = 0x60;
	recv_buf[0] = 0;
	usb_raw_read(handle, OP_RW_ANA, addr, (uint8_t *)recv_buf,2);
	printf("ana 1[%x] = %x\n", addr, recv_buf[0]);

	addr = 0x61;
	recv_buf[0] = 0;
	usb_raw_read(handle, OP_RW_ANA, addr, (uint8_t *)recv_buf,2);
	printf("ana 2[%x] = %x\n", addr, recv_buf[0]);

	recv_buf[1] = recv_buf[0];
	data = recv_buf[0] | 0xF;
	usb_raw_write(handle, OP_RW_ANA, addr, (uint8_t *)&data, 2);
	recv_buf[0] = 0;
	usb_raw_read(handle, OP_RW_ANA, addr, (uint8_t *)recv_buf,2);
	printf("ana 3[%x] = %x\n", addr, recv_buf[0]);

	data = recv_buf[1];
	usb_raw_write(handle, OP_RW_ANA, addr, (uint8_t *)&data, 2);
	recv_buf[0] = 0;
	usb_raw_read(handle, OP_RW_ANA, addr, (uint8_t *)recv_buf,2);
	printf("ana 4[%x] = %x\n", addr, recv_buf[0]);

	// PMU test
	addr = 0x00;
	recv_buf[0] = 0;
	usb_raw_read(handle, OP_RW_PMU, addr, (uint8_t *)recv_buf,2);
	printf("pmu 1[%x] = %x\n", addr, recv_buf[0]);

	addr = 0x01;
	recv_buf[0] = 0;
	usb_raw_read(handle, OP_RW_PMU, addr, (uint8_t *)recv_buf,2);
	printf("pmu 2[%x] = %x\n", addr, recv_buf[0]);

	data = recv_buf[0] | 0x7;
	usb_raw_write(handle, OP_RW_PMU, addr, (uint8_t *)&data, 2);
	recv_buf[0] = 0;
	usb_raw_read(handle, OP_RW_PMU, addr, (uint8_t *)recv_buf,2);
	printf("pmu 3[%x] = %x\n", addr, recv_buf[0]);

	//DIG test
	addr = 0x40300000;
	recv_buf[0] = 0;
	usb_raw_read(handle, OP_RW_DIG, addr, (uint8_t *)recv_buf,4);
	printf("dig 1[%x] = %x\n", addr, recv_buf[0]);

	data = recv_buf[0] | 1;
	usb_raw_write(handle, OP_RW_DIG, addr, (uint8_t *)&data, 4);
	recv_buf[0] = 0;
	usb_raw_read(handle, OP_RW_DIG, addr, (uint8_t *)recv_buf,4);
	printf("dig 2[%x] = %x\n", addr, recv_buf[0]);

	usb_raw_write(handle, OP_RESET_CDC, addr, (uint8_t *)&data, 4);
#if 0
	usb_raw_write(handle, OP_RW_MEM, addr, (uint8_t *)&data, 4);

	rxlen = usb_raw_read(handle, OP_RW_MEM, addr, (uint8_t *)recv_buf, 60);
	data = *(uint32_t *)recv_buf;

	printf("data=%x\n", data);

	for(i = 0; i < rxlen / 4; i++) {
		printf("[%8x] = %08x\n", addr, recv_buf[i]);
		addr += 4;
	}

	addr = 0x6c000000;
	rxlen = usb_raw_read(handle, OP_RW_MEM, addr, (uint8_t *)recv_buf, 60);
	data = *(uint32_t *)recv_buf;

	printf("data=%x\n", data);

	for(i = 0; i < rxlen / 4; i++) {
		printf("[%8x] = %08x\n", addr, recv_buf[i]);
		addr += 4;
	}
#endif

#if 0
	dev = libusb_get_device(handle);

	// get dev desc
	r = libusb_get_device_descriptor(dev, &dev_desc);
	if (r < 0) {
		printf("get dev desc failed %d", r);
		return -1;
	}
#endif

	libusb_exit(NULL);

	return 0;
}

int list_device(void)
{
	libusb_device **devs;
	int r,i;
	ssize_t cnt;

	printf("%s\n", __func__);

	r = libusb_init(NULL);
	if (r) {
		printf("init libusb failed %d\n",r);
		return r;
	}

	cnt = libusb_get_device_list(NULL, &devs);
	if (cnt < 0) {
		printf("get libusb dev failed %d\n",r);
		return -1;
	}
	printf("usb devs cnt=%d\n", (int)cnt);

	struct libusb_device_descriptor desc;
	libusb_device_handle *handle;
	libusb_device *dev;
	unsigned char sbuf[256];

	for(i = 0; devs[i] != NULL; i++) {
		dev = devs[i];

		// get dev desc
		r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0) {
			printf("get dev desc failed %d", r);
			return -1;
		}
		printf("devs[%d]: vid=%04x, pid=%04x\n", i,
			(unsigned int)(desc.idVendor), (unsigned int)(desc.idProduct));

		// open dev
		handle = NULL;
		r = libusb_open(dev, &handle);
		if (r == LIBUSB_SUCCESS) {
			if (desc.iManufacturer) {
				r = libusb_get_string_descriptor_ascii(handle,
					desc.iManufacturer, sbuf, sizeof(sbuf));
				if (r > 0) {
					printf("Manu: %s\n", sbuf);
				}
			}

			if (desc.iProduct) {
				r = libusb_get_string_descriptor_ascii(handle,
					desc.iProduct, sbuf, sizeof(sbuf));
				if (r > 0) {
					printf("Product: %s\n", sbuf);
				}
			}
		} else {
			perror("libusb_open");
			printf("open usb dev failed %d\n", r);
		}

		// get config desc
		struct libusb_config_descriptor *cfg_desc;

		r = libusb_get_config_descriptor(dev, 0, &cfg_desc);
		if (r != LIBUSB_SUCCESS) {
			perror("get config desc");
		} else {
			print_config_desc(cfg_desc);
		}

		//free config desc
		libusb_free_config_descriptor(cfg_desc);


		// close dev
		if (handle) {
			libusb_close(handle);
		}
		printf("\n");
	}
	libusb_free_device_list(devs, 1);

	libusb_exit(NULL);

	return 0;
}

int test_libusb(int argc, char *argv[])
{
	char *cmd = NULL;
	uint16_t vid=0, pid=0;

	if (argc > 1) {
			cmd = argv[1];
	}

	if (cmd == NULL) {
			printf("no cmds\n");
			return -1;
	}

	if (!strcmp(cmd, "-l")) {
		list_device();
	} else if (!strcmp(cmd, "-o")){
		if (argc < 4) {
			printf("no vid pid\n");
			return -1;
		}
		vid = strtoul(argv[2],NULL, 0);
		pid = strtoul(argv[3],NULL, 0);
		printf("vid=%x, pid=%x\n", vid, pid);

		test_device(vid, pid);
	} else {
		printf("unknown cmd\n");
		return -1;
	}
	return 0;
}
#endif
