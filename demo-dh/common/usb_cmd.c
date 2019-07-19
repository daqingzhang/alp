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

#define USB_CMD_GROUP_NAME "usb"

static struct usb_cmd_t cmds_info[] = {
	{"list", USB_CMD_LIST},
	{"bind", USB_CMD_BIND},
	{"ubind",USB_CMD_UNBIND},
	{"read", USB_CMD_READ},
	{"write",USB_CMD_WRITE},
	{"test", USB_CMD_TEST},
};

static struct usb_cmd_t sub_cmds[] = {
	{"ana", USB_MOD_ANA},
	{"pmu", USB_MOD_PMU},
	{"dig", USB_MOD_DIG},
	{"mem", USB_MOD_MEM},
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

void usb_itf_desc_print(const struct libusb_interface_descriptor *alt)
{
	if (!alt)
		return;

	printf("        ==============alt %p===============\n", alt);
	printf("bInterfaceNumber:        %d\n", alt->bInterfaceNumber);
	printf("bAlternateSetting:       %d\n", alt->bAlternateSetting);
	printf("bNumEndpoints:           %d\n", alt->bNumEndpoints);
	printf("        ==============alt %p===============\n", alt);
}

void usb_itf_print(const struct libusb_interface *itf)
{
	int i;

	if (!itf) {
		return;
	}
	printf("    ==============interface %p===============\n", itf);
	printf("num_altsetting: %d\n", itf->num_altsetting);

	for(i = 0; i < itf->num_altsetting; i++) {
		usb_itf_desc_print(itf->altsetting+i);
	}
	printf("    ==============interface %p===============\n", itf);
}

void usb_config_desc_print(struct libusb_config_descriptor *desc)
{
	int i;
	const struct libusb_interface *itf;

	if (!desc) {
		return;
	}
	printf("==============configurator %p===============\n", desc);
	printf("wTotalLength:        %d\n", (int)(desc->wTotalLength));
	printf("bNumInterfaces:      %d\n", desc->bNumInterfaces);
	printf("bConfigurationValue: %d\n", desc->bConfigurationValue);
	printf("bmAttributes:        0x%x\n",desc->bmAttributes);
	printf("MaxPower:            %d\n", desc->MaxPower);

	for(i = 0; i < desc->bNumInterfaces; i++) {
		itf = desc->interface;
		usb_itf_print(itf);
	}
	printf("==============configurator %p===============\n", desc);
}

void usb_device_desc_print(struct libusb_device_descriptor *desc)
{
	if (!desc) {
		return;
	}

	printf("\n==================VID: %04X, PID: %04X=================\n",
		desc->idVendor, desc->idProduct);
	printf("bLength:        %d\n", desc->bLength);
	printf("bDescType:      %d\n", desc->bDescriptorType);
	printf("bcdUSB:         %0x04x\n", desc->bcdUSB); //0x0200: USB-2.0, 0x0110: USB-1.0
	printf("bDeviceClass    %d\n", desc->bDeviceClass);
	printf("bDeviceSubClass %d\n", desc->bDeviceSubClass);
	printf("bDeviceProtocol %d\n", desc->bDeviceProtocol);
	printf("bMaxPacketSize0 %d\n", desc->bMaxPacketSize0);
	printf("idVendor        0x%x\n", desc->idVendor);
	printf("idProduct       0x%x\n", desc->idProduct);
	printf("bcdDevice       0x%x\n", desc->bcdDevice);
	printf("iManu           %d\n",   desc->iManufacturer);
	printf("iProd           %d\n",   desc->iProduct);
	printf("iSeri           %d\n",   desc->iSerialNumber);
	printf("bNrConfig:      %d\n",   desc->bNumConfigurations);
}

static int usb_handle_cmd_list(int argc, char *argv[])
{
	libusb_device **devs;
	struct libusb_device_descriptor desc;
//	libusb_device_handle *handle;
//	unsigned char sbuf[256];
	int r = 0,i;
	ssize_t dev_nr;

	printf("%s\n", __func__);

	r = libusb_init(NULL);
	if (r) {
		printf("init libusb failed %d\n",r);
		return r;
	}

	dev_nr = libusb_get_device_list(NULL, &devs);
	if (dev_nr < 0) {
		printf("get libusb dev failed %d\n",r);
		return -1;
	}
	printf("usb devices number=%d\n", (int)dev_nr);

	for(i = 0; devs[i] != NULL; i++) {
		libusb_device *dev = devs[i];
		//int dev_conf = 0;
		struct libusb_config_descriptor *conf_desc = NULL;
		int bus_num, por_num;
		int dev_addr, dev_speed;

		// get dev desc
		r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0) {
			printf("get dev desc failed %d", r);
			return -1;
		}

		usb_device_desc_print(&desc);

		r = libusb_get_active_config_descriptor(dev, &conf_desc);
		printf("conf_desc = %p, r=%d\n", conf_desc, r);

		bus_num = libusb_get_bus_number(dev);
		por_num = libusb_get_port_number(dev);
		dev_addr = libusb_get_device_address(dev);
		dev_speed = libusb_get_device_speed(dev);
		printf("BUS: %03d, PORT: %03d, DEVICE: %03d, SPEED: %08d\n", bus_num, por_num, dev_addr, dev_speed);

		usb_config_desc_print(conf_desc);

#if 0
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
#endif
	}
	libusb_free_device_list(devs, 1);

	libusb_exit(NULL);

	return 0;
}

static int usb_handle_cmd_bind(int argc, char *argv[], int bind)
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
	USBLOG("%s, vid=%x, pid=%x\n", __func__, vid, pid);

	if (usbctrl.bind == bind) {
		USBLOG("USB bind %d already done", bind);
		return 0;
	}

	if (bind) {
		if (vid == 0 || pid == 0) {
			USBLOG("%s, invalid vid, pid\n", __func__);
			return -1;
		}
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
static int usb_handle_cmd_rw(int argc, char *argv[], int read)
{
#define ANA_REG_MAX_ADDR 0x400

	unsigned int addr = 0, argv3 = 1;
	char *cmd_name = NULL;
	int cmd_id = -1, rlen = 0, i, err = 0;
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
		argv3 = strtoul(argv[USB_IDX_ARGC3], NULL, 0);
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
		unsigned int data = argv3;

		if ((cmd_id == USB_MOD_ANA)
			|| (cmd_id == USB_MOD_PMU)
			|| (cmd_id == USB_MOD_DIG)
			|| (cmd_id == USB_MOD_MEM)) {

			if (argc <= USB_IDX_ARGC3) {
				USBLOG("null data to write\n");
				return -4;
			}
		}

		switch(cmd_id) {
		case USB_MOD_ANA:
		case USB_MOD_PMU:
			if (addr >= ANA_REG_MAX_ADDR) {
				USBLOG("addr %x out of range 0x%x\n", addr, ANA_REG_MAX_ADDR);
				return -3;
			}
			buf[0] = (data & 0xff);
			buf[1] = (data >> 8) & 0xff;
			len = 2;
			break;
		case USB_MOD_DIG:
		case USB_MOD_MEM:
			if (addr & 0x3) {
				USBLOG("unaligned addr %x\n", addr);
				return -3;
			}
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
			return -5;
		}
		rlen = usb_raw_write(usbctrl.handle, cmd_id, addr, buf, len);
		if (rlen != len) {
			USBLOG("write error, rlen=%d, len=%d\n", rlen, len);
			err++;
		}
	} else {
		unsigned int inc = 0;

		switch(cmd_id) {
		case USB_MOD_ANA:
		case USB_MOD_PMU:
			if (addr >= ANA_REG_MAX_ADDR) {
				USBLOG("addr %x out of range 0x%x\n", addr, ANA_REG_MAX_ADDR);
				return -3;
			}
			len = 2;
			inc = 1;
			break;
		case USB_MOD_DIG:
		case USB_MOD_MEM:
			if (addr & 0x3) {
				USBLOG("unaligned addr %x\n", addr);
				return -3;
			}
			len = 4;
			inc = 4;
			break;
		case USB_CMD_EXEC:
		case USB_CMD_RESET:
		case USB_CMD_RESET_CDC:
			len = 4;
			break;
		default:
			return -6;
		}
		memset(buf, 0, sizeof(buf));
		unsigned int times = 0, i = 0;

		if ((cmd_id == USB_MOD_ANA)
			|| (cmd_id == USB_MOD_PMU)
			|| (cmd_id == USB_MOD_DIG)
			|| (cmd_id == USB_MOD_MEM)) {

			times = argv3 * 4 / len;
		}
		for(i = 0; i < times; i++) {
			rlen = usb_raw_read(usbctrl.handle, cmd_id, addr, buf, len);
			if (rlen != len) {
				USBLOG("read error, rlen=%d, len=%d\n", rlen, len);
				err++;
			} else {
				unsigned int d = 0;

				if (len == 4) {
					d = buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
				} else if (len == 2) {
					d = buf[0] | (buf[1] << 8);
				} else {
					d = buf[0];
				}
				USBLOG("DATA: [%x] = %x\n", addr, d);
			}
			addr += inc;
		}
	}
	return err;
}

static int usb_handle_cmd_test(int argc, char *argv[])
{
	int r = 0,i;
	uint16_t vid = 0, pid = 0;
//	libusb_device_handle *handle = NULL;
	libusb_device **devs, *dev;
	ssize_t dev_nr;
	struct libusb_device_descriptor dev_desc;
	struct libusb_config_descriptor *conf_desc = NULL;

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

	r = libusb_init(NULL);
	if (r) {
		printf("init libusb failed %d\n",r);
		return r;
	}

	dev_nr = libusb_get_device_list(NULL, &devs);
	if (dev_nr < 0) {
		printf("get libusb dev failed %d\n",r);
		return -1;
	}
	printf("usb devices number=%d\n", (int)dev_nr);

	dev = NULL;
	for(i = 0; devs[i] != NULL; i++) {
		dev = devs[i];

		r = libusb_get_device_descriptor(dev, &dev_desc);
		if ((dev_desc.idVendor == vid) && (dev_desc.idProduct == pid)) {
			r = libusb_get_active_config_descriptor(dev, &conf_desc);

			usb_device_desc_print(&dev_desc);
			usb_config_desc_print(conf_desc);
			break;
		}
	}

	libusb_free_device_list(devs, 1);

	libusb_exit(NULL);
	return r;
}

static int exec_usb_cmd(int id, int argc, char *argv[])
{
	int r = 0;

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

	USBLOG("%s: %s, %d\n", __func__, (!r)?("OK"):("ERROR"), r);
	return r;
}

static void usb_cmd_handler(int id, void *cdata, void *priv)
{
	struct comm_data *pd = (struct comm_data *)cdata;
	int argc = pd->argc;
	char **argv = (char **)pd->argv;

	USBLOG("%s, id=%d, cdata=%p, priv=%p", __func__, id, cdata, priv);
	exec_usb_cmd(id, argc, argv);
}

int usb_cmd_test_handler(int argc, char *argv[])
{
	int r,id = -1, i;
	const char *group = NULL, *cmd = NULL;

	if (argc > 0) {
		argc--;
		argv++;
	}

	if (argc > USB_IDX_GROUP) {
		group = argv[USB_IDX_GROUP];
	}
	if (argc > USB_IDX_CMD) {
		cmd = argv[USB_IDX_CMD];
	}
	if (!cmd || !group) {
		printf("%s: null cmd or group\n", __func__);
		printf("usb [cmd] [argv1] [argv2] [argv3]\n");
		return -1;
	}

	if (strcmp(USB_CMD_GROUP_NAME, group)) {
		printf("%s: invalid group %s\n", __func__, group);
		return -2;
	}

	for(i = 0; i < ARRAY_SIZE(cmds_info);i++) {
		if (!strcmp(cmds_info[i].name, cmd)) {
			id = cmds_info[i].id;
		}
	}
	if (id < 0) {
		printf("%s: invalid cmd %s\n", __func__, cmd);
		return -3;
	}

	r = exec_usb_cmd(id, argc, argv);
	return r;
}

static struct comm_cmd usb_cmd_list = {
	.id = USB_CMD_LIST,
	.group= USB_CMD_GROUP_NAME,
	.desc = "list usb devices\n"
			"usb list	-	list usb devices\n"
			"\n",
	.cdata = &cdata_usb,
	.priv  = &usbctrl,
	.handler = usb_cmd_handler,
};

static struct comm_cmd usb_cmd_bind = {
	.id = USB_CMD_BIND,
	.group= USB_CMD_GROUP_NAME,
	.desc = "bind one usb device\n"
			"usb bind [vid] [pid]	-	bind usb device\n"
			"\n",
	.cdata = &cdata_usb,
	.priv  = &usbctrl,
	.handler = usb_cmd_handler,
};

static struct comm_cmd usb_cmd_unbind = {
	.id = USB_CMD_UNBIND,
	.group= USB_CMD_GROUP_NAME,
	.desc = "unbind one usb device\n"
			"usb unbind [vid] [pid]	-	unbind usb device\n"
			"\n",
	.cdata = &cdata_usb,
	.priv  = &usbctrl,
	.handler = usb_cmd_handler,
};

static struct comm_cmd usb_cmd_read = {
	.id = USB_CMD_READ,
	.group= USB_CMD_GROUP_NAME,
	.desc = "read usb device\n"
			"usb read [module] [addr] [nr]	-	read usb device\n"
			"\n",
	.cdata = &cdata_usb,
	.priv  = &usbctrl,
	.handler = usb_cmd_handler,
};

static struct comm_cmd usb_cmd_write = {
	.id = USB_CMD_WRITE,
	.group= USB_CMD_GROUP_NAME,
	.desc = "write one usb device\n"
			"write [module] [addr] [value]	-	write usb device\n"
			"\n",
	.cdata = &cdata_usb,
	.priv  = &usbctrl,
	.handler = usb_cmd_handler,
};

static struct comm_cmd usb_cmd_test = {
	.id = USB_CMD_TEST,
	.group= USB_CMD_GROUP_NAME,
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
			cmd->name = cmds_info[cmd->id].name;
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

	if (usbctrl.bind) {
		usb_handle_cmd_bind(0, NULL, 0);
	}
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
