#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>
#include <debug.h>
#include <usb_cmd.h>
#include <comm_cmd.h>

#if 1
#define USBLOG       printf
#else
#define USBLOG(...)  do{}while(0)
#endif

struct usb_message {
	struct libusb_control_setup b;
	uint8_t  *data;
	uint16_t length;
};

struct usb_ctrl_t {
	int bind;
	libusb_device_handle *handle;
	uint16_t vid;
	uint16_t pid;
};

static struct usb_ctrl_t usbctrl;

static struct comm_data cdata_usb;

static struct usb_cmd_t sub_cmds[] = {
	{"ana", USB_MOD_ANA},
	{"pmu", USB_MOD_ANA},
	{"dig", USB_MOD_ANA},
	{"exec", USB_CMD_EXEC},
	{"rst", USB_CMD_RESET},
	{"rstcdc", USB_CMD_RESET_CDC},
};

#define REQ_TYPE_WR (LIBUSB_ENDPOINT_OUT \
		   | LIBUSB_REQUEST_TYPE_VENDOR \
		   | LIBUSB_RECIPIENT_OTHER)

#define REQ_TYPE_RD (LIBUSB_ENDPOINT_IN \
		   | LIBUSB_REQUEST_TYPE_VENDOR \
		   | LIBUSB_RECIPIENT_OTHER)

static int usb_raw_write(libusb_device_handle *h, uint8_t opcode,
		uint32_t addr, uint8_t *pbuf, uint32_t len)
{
	int r;
	struct usb_message msg;

	USBLOG("%s, opcode=%x, addr=%x, len=%d\n", __func__,
		opcode, addr, len);

	for(int i = 0; i < len; i++) {
		USBLOG("write data[%d]=%2x\n", i, pbuf[i]);
	}

	msg.b.bmRequestType = REQ_TYPE_WR;
	msg.b.bRequest      = opcode;
	msg.b.wValue        = addr & 0xFFFF;
	msg.b.wIndex        = addr >> 16;
	msg.b.wLength       = (uint16_t)len;
	msg.data            = pbuf;

	r = libusb_control_transfer(h, msg.b.bmRequestType,
		msg.b.bRequest, msg.b.wValue, msg.b.wIndex,
		msg.data, msg.b.wLength, 1000);

	if (r < 0) {
		perror("write usb dev");
	}
	USBLOG("r=%d\n", r);
	return r;
}

static int usb_raw_read(libusb_device_handle *h, uint8_t opcode,
		uint32_t addr, uint8_t *pbuf, uint32_t len)
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
	msg.data            = pbuf;

	r = libusb_control_transfer(h, msg.b.bmRequestType,
		msg.b.bRequest, msg.b.wValue, msg.b.wIndex,
		msg.data, msg.b.wLength, 1000);

	if (r < 0) {
		perror("read usb dev");
	}
	for(int i = 0; i < r; i++) {
		USBLOG("get data[%d]=%2x\n", i, pbuf[i]);
	}
	USBLOG("r=%d\n", r);
	return r;
}

static void print_config_desc(struct libusb_config_descriptor *desc)
{
	if (!desc) {
		return;
	}
	printf("config desc info: \n");
	printf("bmAttrbutes: %d\n", desc->bmAttributes);
	printf("MaxPower:    %d\n", desc->MaxPower);
}

static int usb_handle_cmd_list(int argc, void *argv[])
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

static int usb_handle_cmd_bind(int argc, void *argv[], int bind)
{
	int r;
	uint16_t vid = 0, pid = 0;
	libusb_device_handle *handle = NULL;

	if (argc > USB_IDX_ARGC1) {
		vid = strtoul(argv[USB_IDX_ARGC1], NULL, 0);
	}
	if (argc > USB_IDX_ARGC2) {
		pid = strtoul(argv[USB_IDX_ARGC2], NULL, 0);
	}
	if (vid == 0 || pid == 0) {
		USBLOG("%s, invalid vid, pid\n", __func__);
		return -1;
	}
	USBLOG("%s, vid=%x, pid=%x\n", __func__, vid, pid);

	if (usbctrl.bind == bind) {
		USBLOG("USB bind %d already done", bind);
		return 0;
	}

	if (bind) {
		r = libusb_init(NULL);
		if (r) {
			USBLOG("init usb failed %d", r);
			return -1;
		}
		handle = libusb_open_device_with_vid_pid(NULL, vid, pid);
		if (!handle) {
			USBLOG("open libusb failed");
			return -1;
		}
		USBLOG("open usb okay, vid=%x, pid=%x\n", vid, pid);
	} else {
		if (usbctrl.handle) {
			libusb_close(usbctrl.handle);
		}
		handle = NULL;
		USBLOG("close usb okay, vid=%x, pid=%x\n", vid, pid);
	}
	usbctrl.vid = vid;
	usbctrl.pid = pid;
	usbctrl.handle = handle;
	usbctrl.bind = bind;

	return 0;
}

// usb rd [mod] [addr] [nr]
// usb wr [mod] [addr] [data]
static int usb_handle_cmd_rw(int argc, void *argv[], int read)
{
	unsigned int addr = 0, data = 0;
	char *cmd_name = NULL;
	int cmd_id = -1, retval = 0, i;
	uint8_t buf[8], len;

	if(!usbctrl.bind) {
		USBLOG("usb not bind\n");
		return -1;
	}

	if (argc > USB_IDX_ARGC1) {
		cmd_name = argv[USB_IDX_ARGC1];
	}
	if (argc > USB_IDX_ARGC2) {
		addr = strtoul(argv[USB_IDX_ARGC2], NULL, 0);
	}
	if (argc > USB_IDX_ARGC3) {
		data = strtoul(argv[USB_IDX_ARGC3], NULL, 0);
	}

	if (cmd_name == NULL) {
		USBLOG("invalid cmd\n");
		return -2;
	}
	for (i = 0; i < ARRAY_SIZE(sub_cmds); i++) {
		if (!strcmp(sub_cmds[i].name, cmd_name)) {
			cmd_id = sub_cmds[i].id;
			break;
		}
	}
	if (cmd_id < 0) {
		USBLOG("unsupported cmd: %s\n", cmd_name);
		return -3;
	}
	if (!read) {
		switch(cmd_id) {
		case USB_MOD_ANA:
		case USB_MOD_PMU:
			buf[0] = (data & 0xff);
			buf[1] = (data >> 8) & 0xff;
			len = 2;
			break;
		case USB_MOD_DIG:
		case USB_MOD_MEM:
			buf[0] = (data & 0xff);
			buf[1] = (data >> 8) & 0xff;
			buf[2] = (data >> 16) & 0xff;
			buf[3] = (data >> 24) & 0xff;
			len = 4;
			break;
		case USB_CMD_EXEC:
		case USB_CMD_RESET:
		case USB_CMD_RESET_CDC:
			buf[0] = 0;
			buf[1] = 0;
			buf[2] = 0;
			buf[3] = 0;
			len = 4;
			break;
		default:
			return -3;
		}
		retval = usb_raw_write(usbctrl.handle, cmd_id, addr, buf, len);
	} else {
		switch(cmd_id) {
		case USB_MOD_ANA:
		case USB_MOD_PMU:
			len = 2;
			break;
		case USB_MOD_DIG:
		case USB_MOD_MEM:
		case USB_CMD_EXEC:
		case USB_CMD_RESET:
		case USB_CMD_RESET_CDC:
			len = 4;
			break;
		default:
			return -4;
		}
		memset(buf, 0, sizeof(buf));
		retval = usb_raw_read(usbctrl.handle, cmd_id, addr, buf, len);
	}
	return retval;
}

static int usb_handle_cmd_test(int argc, void *argv[])
{
	USBLOG("%s \n", __func__);
	return 0;
}

static void usb_cmd_handler(int id, void *cdata, void *priv)
{
	int r;
	struct comm_data *pd = (struct comm_data *)cdata;
	int argc = pd->argc;
	void **argv = (void **)pd->argv;

	USBLOG("%s, id=%d, cdata=%p, priv=%p", __func__, id, cdata, priv);

	switch(id) {
	case USB_CMD_LIST:
		r = usb_handle_cmd_list(argc, argv);
		break;
	case USB_CMD_BIND:
		r = usb_handle_cmd_bind(argc, argv, 1);
		break;
	case USB_CMD_UNBIND:
		r = usb_handle_cmd_bind(argc, argv, 0);
		break;
	case USB_CMD_READ:
		r = usb_handle_cmd_rw(argc, argv, 1);
		break;
	case USB_CMD_WRITE:
		r = usb_handle_cmd_rw(argc, argv, 0);
		break;
	case USB_CMD_TEST:
		r = usb_handle_cmd_test(argc, argv);
		break;
	default:
		break;
	}
	if (r) {
		USBLOG("%s, error %d\n", __func__, r);
	} else {
		USBLOG("%s, success\n", __func__);
	}
}

static struct comm_cmd usb_cmd_list = {
	.id = USB_CMD_LIST,
	.name = "list",
	.group= "usb",
	.desc = "list usb devices\n"
			"usb list	-	list usb devices\n"
			"\n",
	.cdata = &cdata_usb,
	.priv  = &usbctrl,
	.handler = usb_cmd_handler,
};

static struct comm_cmd usb_cmd_bind = {
	.id = USB_CMD_BIND,
	.name = "bind",
	.group= "usb",
	.desc = "bind one usb device\n"
			"usb bind [vid] [pid]	-	bind usb device\n"
			"\n",
	.cdata = &cdata_usb,
	.priv  = &usbctrl,
	.handler = usb_cmd_handler,
};

static struct comm_cmd usb_cmd_unbind = {
	.id = USB_CMD_UNBIND,
	.name = "unbind",
	.group= "usb",
	.desc = "unbind one usb device\n"
			"usb unbind [vid] [pid]	-	unbind usb device\n"
			"\n",
	.cdata = &cdata_usb,
	.priv  = &usbctrl,
	.handler = usb_cmd_handler,
};

static struct comm_cmd usb_cmd_read = {
	.id = USB_CMD_READ,
	.name = "read",
	.group= "usb",
	.desc = "read usb device\n"
			"usb read [module] [addr] [nr]	-	read usb device\n"
			"\n",
	.cdata = &cdata_usb,
	.priv  = &usbctrl,
	.handler = usb_cmd_handler,
};

static struct comm_cmd usb_cmd_write = {
	.id = USB_CMD_WRITE,
	.name = "write",
	.group= "usb",
	.desc = "write one usb device\n"
			"write [module] [addr] [value]	-	write usb device\n"
			"\n",
	.cdata = &cdata_usb,
	.priv  = &usbctrl,
	.handler = usb_cmd_handler,
};

static struct comm_cmd usb_cmd_test = {
	.id = USB_CMD_WRITE,
	.name = "test",
	.group= "usb",
	.desc = "test usb device\n"
			"test -	test usb device\n"
			"\n",
	.cdata = &cdata_usb,
	.priv  = &usbctrl,
	.handler = usb_cmd_handler,
};

static struct comm_cmd *usb_cmds[] = {
	&usb_cmd_list,
	&usb_cmd_bind,
	&usb_cmd_unbind,
	&usb_cmd_read,
	&usb_cmd_write,
	&usb_cmd_test,
};

static void usb_data_init(void)
{
	memset(&usbctrl, 0, sizeof(usbctrl));
}

int usb_cmd_register(void)
{
	int i, r=0;
	struct comm_cmd *cmd;

	usb_data_init();

	for(i = 0; i < ARRAY_SIZE(usb_cmds); i++) {
		cmd = usb_cmds[i];
		if (cmd) {
			comm_data_init(cmd->cdata);
			r = comm_cmd_register(cmd);
			if (r) {
				ioprintf("register cmd %s failed, %d\n",
					cmd->name, r);
				break;
			}
		}
	}
	return r;
}

int usb_cmd_unregister(void)
{
	int i, r=0;
	struct comm_cmd *cmd;

	for(i = 0; i < ARRAY_SIZE(usb_cmds); i++) {
		cmd = usb_cmds[i];
		if (cmd) {
			r = comm_cmd_unregister(cmd);
			if (r) {
				ioprintf("unregister cmd %s failed, %d\n",
					cmd->name, r);
				break;
			}
		}
	}
	return r;
}
