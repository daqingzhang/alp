#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>
#include <debug.h>
#include <usb_cmd.h>
#include <comm_cmd.h>

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
	{"list1", USB_CMD_LIST1},
	{"bind", USB_CMD_BIND},
	{"unbind",USB_CMD_UNBIND},
	{"read", USB_CMD_READ},
	{"write",USB_CMD_WRITE},
	{"test", USB_CMD_TEST},
	{"fm", USB_CMD_FM},
};

// memory access commands
static struct usb_cmd_t rw_sub_cmds[] = {
	{"ana", USB_MOD_ANA},
	{"pmu", USB_MOD_PMU},
	{"dig", USB_MOD_DIG},
	{"mem", USB_MOD_MEM},
	{"exec", USB_CMD_EXEC},
	{"rst", USB_CMD_RESET},
	{"rstcdc", USB_CMD_RESET_CDC},
};

// FM sub commands
static struct usb_cmd_t fm_sub_cmds[] = {
	{"on",      USB_FM_CMD_POWER_ON},
	{"off",     USB_FM_CMD_POWER_OFF},
	{"volume",  USB_FM_CMD_VOLUME},
	{"channel", USB_FM_CMD_CHANNEL},
	{"band",    USB_FM_CMD_BAND},
	{"device",  USB_FM_CMD_DEVICE},
	{"test",    USB_FM_CMD_TEST},
};

#define REQ_TYPE_OTH_WR (LIBUSB_ENDPOINT_OUT \
		   | LIBUSB_REQUEST_TYPE_VENDOR \
		   | LIBUSB_RECIPIENT_OTHER)

#define REQ_TYPE_OTH_RD (LIBUSB_ENDPOINT_IN \
		   | LIBUSB_REQUEST_TYPE_VENDOR \
		   | LIBUSB_RECIPIENT_OTHER)

static int usb_raw_write(libusb_device_handle *h, uint8_t request, uint8_t opcode,
		uint32_t addr, uint8_t *pbuf, uint32_t len)
{
	int r;
	struct usb_message msg;

	USB_TRACE(1, "%s, opcode=%x, addr=%x, len=%d\n", __func__,
		opcode, addr, len);

	for(int i = 0; i < len; i++) {
		USB_TRACE(1, "write data[%d]=%2x\n", i, pbuf[i]);
	}

	msg.b.bmRequestType = request;
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
	USB_TRACE(1, "r=%d\n", r);
	return r;
}

static int usb_raw_read(libusb_device_handle *h, uint8_t request, uint8_t opcode,
		uint32_t addr, uint8_t *pbuf, uint32_t len)
{
	int r;
	struct usb_message msg;

	USB_TRACE(1, "%s, opcode=%x, addr=%x, len=%d\n", __func__,
		opcode, addr, len);

	msg.b.bmRequestType = request;
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
		USB_TRACE(1, "get data[%d]=%2x\n", i, pbuf[i]);
	}
	USB_TRACE(1, "r=%d\n", r);
	return r;
}

void usb_itf_desc_print(const struct libusb_interface_descriptor *alt)
{
	if (!alt)
		return;

	USB_TRACE(2, "        ==============alt %p===============\n", alt);
	USB_TRACE(2, "bInterfaceNumber:        %d\n", alt->bInterfaceNumber);
	USB_TRACE(2, "bAlternateSetting:       %d\n", alt->bAlternateSetting);
	USB_TRACE(2, "bNumEndpoints:           %d\n", alt->bNumEndpoints);
	USB_TRACE(2, "        ==============alt %p===============\n", alt);
}

void usb_itf_print(const struct libusb_interface *itf)
{
	int i;

	if (!itf) {
		return;
	}
	USB_TRACE(2, "    ==============interface %p===============\n", itf);
	USB_TRACE(2, "num_altsetting: %d\n", itf->num_altsetting);

	for(i = 0; i < itf->num_altsetting; i++) {
		usb_itf_desc_print(itf->altsetting+i);
	}
	USB_TRACE(2, "    ==============interface %p===============\n", itf);
}

void usb_config_desc_print(struct libusb_config_descriptor *desc)
{
	int i;
	const struct libusb_interface *itf;

	if (!desc) {
		return;
	}
	USB_TRACE(2, "==============configurator %p===============\n", desc);
	USB_TRACE(2, "wTotalLength:        %d\n", (int)(desc->wTotalLength));
	USB_TRACE(2, "bNumInterfaces:      %d\n", desc->bNumInterfaces);
	USB_TRACE(2, "bConfigurationValue: %d\n", desc->bConfigurationValue);
	USB_TRACE(2, "bmAttributes:        0x%x\n",desc->bmAttributes);
	USB_TRACE(2, "MaxPower:            %d\n", desc->MaxPower);

	for(i = 0; i < desc->bNumInterfaces; i++) {
		itf = desc->interface;
		usb_itf_print(itf);
	}
	USB_TRACE(2, "==============configurator %p===============\n", desc);
}

void usb_device_desc_print(struct libusb_device_descriptor *desc)
{
	uint8_t *pbuf;
	uint8_t len, i;

	if (!desc) {
		return;
	}

	USB_TRACE(2, "\n=======================================================\n");
	USB_TRACE(2, "Device Descriptor:\n");

	len = desc->bLength;
	pbuf = (uint8_t *)desc;
	for(i = 0; i < len; i++) {
		USB_TRACE(2, "%02X ", pbuf[i]);
		if ((i+1)%18 == 0)
			USB_TRACE(2, "\n");
	}
#if 0
	USB_TRACE(2, "bLength:        %d\n", desc->bLength);
	USB_TRACE(2, "bDescType:      %d\n", desc->bDescriptorType);
	USB_TRACE(2, "bcdUSB:         %0x04x\n", desc->bcdUSB);
	USB_TRACE(2, "bDeviceClass    %d\n", desc->bDeviceClass);
	USB_TRACE(2, "bDeviceSubClass %d\n", desc->bDeviceSubClass);
	USB_TRACE(2, "bDeviceProtocol %d\n", desc->bDeviceProtocol);
	USB_TRACE(2, "bMaxPacketSize0 %d\n", desc->bMaxPacketSize0);
	USB_TRACE(2, "idVendor        0x%x\n", desc->idVendor);
	USB_TRACE(2, "idProduct       0x%x\n", desc->idProduct);
	USB_TRACE(2, "bcdDevice       0x%x\n", desc->bcdDevice);
	USB_TRACE(2, "iManu           %d\n",   desc->iManufacturer);
	USB_TRACE(2, "iProd           %d\n",   desc->iProduct);
	USB_TRACE(2, "iSeri           %d\n",   desc->iSerialNumber);
	USB_TRACE(2, "bNrConfig:      %d\n",   desc->bNumConfigurations);
#endif
}

static int usb_handle_cmd_list(int argc, char *argv[], int level)
{
	libusb_device **devs;
	struct libusb_device_descriptor desc;
//	libusb_device_handle *handle;
//	unsigned char sbuf[256];
	int r = 0,i;
	ssize_t dev_nr;

	USB_TRACE(1, "%s\n", __func__);

	r = libusb_init(NULL);
	if (r) {
		USB_TRACE(0, "init libusb failed %d\n",r);
		return r;
	}

	dev_nr = libusb_get_device_list(NULL, &devs);
	if (dev_nr < 0) {
		USB_TRACE(0, "get libusb dev failed %d\n",r);
		return -1;
	}
	USB_TRACE(2, "usb devices number=%d\n", (int)dev_nr);

	for(i = 0; devs[i] != NULL; i++) {
		libusb_device *dev = devs[i];
		//int dev_conf = 0;
		struct libusb_config_descriptor *conf_desc = NULL;
		int bus_num = 0, por_num = 0;
		int dev_addr = 0, dev_speed = 0;

		dev_addr = dev_addr;
		dev_speed= dev_speed;
		bus_num = bus_num;
		por_num = por_num;

		// get dev desc
		r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0) {
			USB_TRACE(0, "get dev desc failed %d", r);
			return -1;
		}

		USB_TRACE(2, "VID: %04X, PID: %04X\n",desc.idVendor, desc.idProduct);
		if (level == 1) {
			usb_device_desc_print(&desc);
		}

		r = libusb_get_active_config_descriptor(dev, &conf_desc);
		USB_TRACE(1, "conf_desc = %p, r=%d\n", conf_desc, r);

		bus_num = libusb_get_bus_number(dev);
		por_num = libusb_get_port_number(dev);
		dev_addr = libusb_get_device_address(dev);
		dev_speed = libusb_get_device_speed(dev);
		USB_TRACE(2, "BUS: %03d, PORT: %03d, DEVICE: %03d, SPEED: %08d\n\n",
			bus_num, por_num, dev_addr, dev_speed);

		if (level == 1) {
			usb_config_desc_print(conf_desc);
		}

#if 0
		// open dev
		handle = NULL;
		r = libusb_open(dev, &handle);
		if (r == LIBUSB_SUCCESS) {
			if (desc.iManufacturer) {
				r = libusb_get_string_descriptor_ascii(handle,
					desc.iManufacturer, sbuf, sizeof(sbuf));
				if (r > 0) {
					USB_TRACE("Manu: %s\n", sbuf);
				}
			}

			if (desc.iProduct) {
				r = libusb_get_string_descriptor_ascii(handle,
					desc.iProduct, sbuf, sizeof(sbuf));
				if (r > 0) {
					USB_TRACE("Product: %s\n", sbuf);
				}
			}
		} else {
			perror("libusb_open");
			USB_TRACE("open usb dev failed %d\n", r);
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
		USB_TRACE("\n");
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
	USB_TRACE(1, "%s, vid=%x, pid=%x\n", __func__, vid, pid);

	if (usbctrl.bind == bind) {
		USB_TRACE(2, "USB bind %d already done", bind);
		return 0;
	}

	if (bind) {
		if (vid == 0 || pid == 0) {
			USB_TRACE(0, "%s, invalid vid, pid\n", __func__);
			return -1;
		}
		r = libusb_init(NULL);
		if (r) {
			USB_TRACE(0, "init usb failed %d", r);
			return -1;
		}
		handle = libusb_open_device_with_vid_pid(NULL, vid, pid);
		if (!handle) {
			USB_TRACE(0, "open libusb failed\n");
			return -1;
		}
		USB_TRACE(2, "open usb okay, vid=%x, pid=%x\n", vid, pid);
	} else {
		if (usbctrl.handle) {
			libusb_close(usbctrl.handle);
		}
		handle = NULL;
		USB_TRACE(2, "close usb okay, vid=%x, pid=%x\n", vid, pid);
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
		USB_TRACE(0, "usb not bind\n");
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
		USB_TRACE(0, "invalid cmd\n");
		return -2;
	}
	for (i = 0; i < ARRAY_SIZE(rw_sub_cmds); i++) {
		if (!strcmp(rw_sub_cmds[i].name, cmd_name)) {
			cmd_id = rw_sub_cmds[i].id;
			break;
		}
	}
	if (cmd_id < 0) {
		USB_TRACE(0, "unsupported cmd: %s\n", cmd_name);
		return -3;
	}
	if (!read) {
		unsigned int data = argv3;

		if ((cmd_id == USB_MOD_ANA)
			|| (cmd_id == USB_MOD_PMU)
			|| (cmd_id == USB_MOD_DIG)
			|| (cmd_id == USB_MOD_MEM)) {

			if (argc <= USB_IDX_ARGC3) {
				USB_TRACE(0, "null data to write\n");
				return -4;
			}
		}

		switch(cmd_id) {
		case USB_MOD_ANA:
		case USB_MOD_PMU:
			if (addr >= ANA_REG_MAX_ADDR) {
				USB_TRACE(0, "addr %x out of range 0x%x\n", addr, ANA_REG_MAX_ADDR);
				return -3;
			}
			buf[0] = (data & 0xff);
			buf[1] = (data >> 8) & 0xff;
			len = 2;
			break;
		case USB_MOD_DIG:
		case USB_MOD_MEM:
			if (addr & 0x3) {
				USB_TRACE(0, "unaligned addr %x\n", addr);
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
		rlen = usb_raw_write(usbctrl.handle, REQ_TYPE_OTH_WR, cmd_id, addr, buf, len);
		if (rlen != len) {
			USB_TRACE(0, "write error, rlen=%d, len=%d\n", rlen, len);
			err++;
		}
	} else {
		unsigned int inc = 0;

		switch(cmd_id) {
		case USB_MOD_ANA:
		case USB_MOD_PMU:
			if (addr >= ANA_REG_MAX_ADDR) {
				USB_TRACE(0, "addr %x out of range 0x%x\n",
						addr, ANA_REG_MAX_ADDR);
				return -3;
			}
			len = 2;
			inc = 1;
			break;
		case USB_MOD_DIG:
		case USB_MOD_MEM:
			if (addr & 0x3) {
				USB_TRACE(0, "unaligned addr %x\n", addr);
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
			rlen = usb_raw_read(usbctrl.handle, REQ_TYPE_OTH_RD, cmd_id, addr, buf, len);
			if (rlen != len) {
				USB_TRACE(0, "read error, rlen=%d, len=%d\n", rlen, len);
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
				USB_TRACE(2, "DATA: [%x] = %x\n", addr, d);
			}
			addr += inc;
		}
	}
	return err;
}

int usb_fm_raw_xfer(libusb_device_handle *h, uint8_t type, uint8_t request,
	uint16_t wValue, uint16_t wIndex, uint16_t wLength, uint8_t *pbuf)
{
	int r;
	struct usb_message msg;

	msg.b.bmRequestType = type;
	msg.b.bRequest      = request;
	msg.b.wValue        = wValue;
	msg.b.wIndex        = wIndex;
	msg.b.wLength       = wLength;
	msg.data            = pbuf;

	r = libusb_control_transfer(h, msg.b.bmRequestType,
		msg.b.bRequest, msg.b.wValue, msg.b.wIndex,
		msg.data, msg.b.wLength, 1000);

	if (r < 0) {
		perror("write usb dev");
		USB_TRACE(0, "%s, error %d\n", __func__,r);
		return 0;
	}
	USB_TRACE(1, "r=%d\n", r);
	return r;
}

#include <usb_fm_ctrl.h>
#define REQ_TYPE_DEV_WR (LIBUSB_ENDPOINT_OUT \
		   | LIBUSB_REQUEST_TYPE_VENDOR \
		   | LIBUSB_RECIPIENT_DEVICE)

#define REQ_TYPE_DEV_RD (LIBUSB_ENDPOINT_IN \
		   | LIBUSB_REQUEST_TYPE_VENDOR \
		   | LIBUSB_RECIPIENT_DEVICE)

int usb_fm_cmd_power(int on)
{
	uint8_t err = 0;

	on = on ? 1 : 0;
	//*handle, type, request, wValue, wIndex, wLength, *pbuf
	usb_fm_raw_xfer(usbctrl.handle, REQ_TYPE_DEV_RD,
		USB_FM_PRIMITIVE_SET, USB_FM_SET_POWER, on, 1, &err);

	USB_TRACE(2, "%s on=%d\n", __func__, on);
	return err;
}

int usb_fm_cmd_band(uint16_t *band, int set)
{
	uint16_t b = 0;
	uint8_t err = 0;

	//*handle, type, request, wValue, wIndex, wLength, *pbuf
	if (set) {
		b = *band & 0x03;
		usb_fm_raw_xfer(usbctrl.handle, REQ_TYPE_DEV_RD,
			USB_FM_PRIMITIVE_SET, USB_FM_SET_BAND, b, 1, &err);
	} else {
		usb_fm_raw_xfer(usbctrl.handle, REQ_TYPE_DEV_RD,
			USB_FM_PRIMITIVE_SET, USB_FM_GET_BAND, 0, 2, (uint8_t *)&b);
		if (band)
			*band = b;
	}
	USB_TRACE(2, "%s, band=%d\n", __func__, b);
	return err;
}

int usb_fm_cmd_channel(uint16_t *chan, int set)
{
	uint8_t err = 0;
	uint16_t ch = 0;

	if (set) {
		ch = *chan;
		usb_fm_raw_xfer(usbctrl.handle, REQ_TYPE_DEV_RD,
			USB_FM_PRIMITIVE_SET, USB_FM_SET_TUNING_CHAN, ch, 1, &err);
	} else {
		usb_fm_raw_xfer(usbctrl.handle, REQ_TYPE_DEV_RD,
			USB_FM_PRIMITIVE_GET, USB_FM_GET_CUR_CHAN, 0, 2, (uint8_t *)&ch);
		if (chan)
			*chan = ch;
	}
	USB_TRACE(2, "%s, chan=%d\n", __func__, ch);
	return err;
}

int usb_fm_cmd_volume(uint16_t *vol, int set)
{
	uint8_t err = 0;
	uint16_t v = 0;

	if (set) {
		v = *vol & 0xF;
		usb_fm_raw_xfer(usbctrl.handle, REQ_TYPE_DEV_RD,
			USB_FM_PRIMITIVE_SET, USB_FM_SET_VOLUME, v, 1, &err);
	} else {
		usb_fm_raw_xfer(usbctrl.handle, REQ_TYPE_DEV_RD,
			USB_FM_PRIMITIVE_GET, USB_FM_GET_VOL, 0, 2, (uint8_t *)&v);
		if(vol)
			*vol = v;
	}
	USB_TRACE(2, "%s, vol=%d\n", __func__, v);
	return err;
}

int usb_fm_cmd_get_device(uint16_t *devid)
{
	uint8_t err = 0;

	usb_fm_raw_xfer(usbctrl.handle, REQ_TYPE_DEV_RD,
		USB_FM_PRIMITIVE_GET, USB_FM_GET_FM_IC, 0, 2, (uint8_t *)devid);

	USB_TRACE(2, "%s, id=%d\n", __func__, *devid);
	return err;
}

int usb_fm_cmd_test(void)
{
	uint16_t devid = 0;
	uint16_t chan = 10390, vol = 8, band = 0;

	usb_fm_cmd_get_device(&devid);
	usleep(10*1000);
	usb_fm_cmd_power(1);
	usleep(10*1000);
	usb_fm_cmd_band(&band, 1);
	usleep(10*1000);
	usb_fm_cmd_channel(&chan, 1);
	usleep(10*1000);
	usb_fm_cmd_volume(&vol, 1);
	usleep(10*1000);

	return 0;
}

//usb fm [cmd] [arg]
static int usb_handle_cmd_fm(int argc, char *argv[])
{
	char *sub_cmd = NULL;
	uint16_t arg = 0;
	int is_set = 0, i, id = -1, r = 0;

	if (!usbctrl.bind) {
		USB_TRACE(0, "usb not bind\n");
		return -1;
	}
	if (argc > USB_IDX_ARGC1) {
		sub_cmd = argv[USB_IDX_ARGC1];
	}
	if (argc > USB_IDX_ARGC2) {
		arg = strtoul(argv[USB_IDX_ARGC2],NULL,0);
		is_set = 1;
	}
	if (sub_cmd == NULL) {
		USB_TRACE(0, "null cmd\n");
		return -2;
	}
	for(i = 0; i < ARRAY_SIZE(fm_sub_cmds); i++) {
		if (!strcmp(sub_cmd, fm_sub_cmds[i].name)) {
			id = fm_sub_cmds[i].id;
		}
	}
	if (id < 0) {
		USB_TRACE(0, "unsupported cmd: %s\n", sub_cmd);
		return -3;
	}

	uint16_t chan = 0, vol = 0;
	uint16_t band = 0;
	uint16_t dev_id = 0;

	switch(id) {
	case USB_FM_CMD_POWER_ON:
		r = usb_fm_cmd_power(1);
		break;
	case USB_FM_CMD_POWER_OFF:
		r = usb_fm_cmd_power(0);
		break;
	case USB_FM_CMD_VOLUME:
		if (is_set) {
			vol= arg;
		}
		r = usb_fm_cmd_volume(&vol, is_set);
		break;
	case USB_FM_CMD_CHANNEL:
		if (is_set) {
			chan = arg;
		}
		r = usb_fm_cmd_channel(&chan, is_set);
		break;
	case USB_FM_CMD_BAND:
		if (is_set) {
			band = arg;
		}
		r = usb_fm_cmd_band(&band, is_set);
		break;
	case USB_FM_CMD_DEVICE:
		r = usb_fm_cmd_get_device(&dev_id);
		break;
	case USB_FM_CMD_TEST:
		r = usb_fm_cmd_test();
	default:
		return -4;
	}

	USB_TRACE(1, "usb fm %s: done, err=%d\n", fm_sub_cmds[id].name, r);
	return 0;
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
		USB_TRACE(0, "%s, invalid vid, pid\n", __func__);
		return -1;
	}
	USB_TRACE(2, "%s, vid=%x, pid=%x\n", __func__, vid, pid);

	r = libusb_init(NULL);
	if (r) {
		USB_TRACE(0, "init libusb failed %d\n",r);
		return r;
	}

	dev_nr = libusb_get_device_list(NULL, &devs);
	if (dev_nr < 0) {
		USB_TRACE(0, "get libusb dev failed %d\n",r);
		return -1;
	}
	USB_TRACE(2, "usb devices number=%d\n", (int)dev_nr);

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
		r = usb_handle_cmd_list(argc, argv, 0);
		break;
	case USB_CMD_LIST1:
		r = usb_handle_cmd_list(argc, argv, 1);
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
	case USB_CMD_FM:
		r = usb_handle_cmd_fm(argc, argv);
		break;
	default:
		break;
	}

	USB_TRACE(1, "%s: %s, %d\n", __func__, (!r)?("OK"):("ERROR"), r);
	return r;
}

static void usb_cmd_handler(int id, void *cdata, void *priv)
{
	struct comm_data *pd = (struct comm_data *)cdata;
	int argc = pd->argc;
	char **argv = (char **)pd->argv;

	USB_TRACE(1, "%s, id=%d, cdata=%p, priv=%p", __func__, id, cdata, priv);
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
		USB_TRACE(0, "%s: null cmd or group\n", __func__);
		USB_TRACE(0, "usb [cmd] [argv1] [argv2] [argv3]\n");
		return -1;
	}

	if (strcmp(USB_CMD_GROUP_NAME, group)) {
		USB_TRACE(0,"%s: invalid group %s\n", __func__, group);
		return -2;
	}

	for(i = 0; i < ARRAY_SIZE(cmds_info);i++) {
		if (!strcmp(cmds_info[i].name, cmd)) {
			id = cmds_info[i].id;
		}
	}
	if (id < 0) {
		USB_TRACE(0, "%s: invalid cmd %s\n", __func__, cmd);
		return -3;
	}

	r = exec_usb_cmd(id, argc, argv);
	return r;
}

static struct comm_cmd usb_cmd_list = {
	.id = USB_CMD_LIST,
	.group= USB_CMD_GROUP_NAME,
	.desc = "list usb devices\n"
			"usage: usb list\n"
			"\n",
	.cdata = &cdata_usb,
	.priv  = &usbctrl,
	.handler = usb_cmd_handler,
};

static struct comm_cmd usb_cmd_list1 = {
	.id = USB_CMD_LIST1,
	.group= USB_CMD_GROUP_NAME,
	.desc = "list usb devices with more informations\n"
			"usage: usb list1\n"
			"\n",
	.cdata = &cdata_usb,
	.priv  = &usbctrl,
	.handler = usb_cmd_handler,
};

static struct comm_cmd usb_cmd_bind = {
	.id = USB_CMD_BIND,
	.group= USB_CMD_GROUP_NAME,
	.desc = "bind one usb device\n"
			"usage: usb bind [vid] [pid]\n"
			"example:\n"
			"    usb bind 0xbe57 0x020f    - bind the usb device(vid=0xbe57, pid=0x020f)\n"
			"\n",
	.cdata = &cdata_usb,
	.priv  = &usbctrl,
	.handler = usb_cmd_handler,
};

static struct comm_cmd usb_cmd_unbind = {
	.id = USB_CMD_UNBIND,
	.group= USB_CMD_GROUP_NAME,
	.desc = "unbind one usb device\n"
			"usage usb unbind [vid] [pid]\n"
			"example:\n"
			"    usb unbind 0xbe57 0x020f  - unbind the usb device(vid=0xbe57, pid=0x020f)\n"
			"\n",
	.cdata = &cdata_usb,
	.priv  = &usbctrl,
	.handler = usb_cmd_handler,
};

static struct comm_cmd usb_cmd_read = {
	.id = USB_CMD_READ,
	.group= USB_CMD_GROUP_NAME,
	.desc = "read usb device\n"
			"usage: usb read [module] [addr] [nr]\n"
			"example:\n"
			"    usb read ana 0x60          - read analog [0x60]\n"
			"    usb read pmu 0x00          - read pmu [0x00]\n"
			"    usb read dig 0x40300060    - read digital [0x40300060]\n"
			"    usb read mem 0x20000000 8  - read memory [0x20000000], 8 words\n"
			"\n",
	.cdata = &cdata_usb,
	.priv  = &usbctrl,
	.handler = usb_cmd_handler,
};

static struct comm_cmd usb_cmd_write = {
	.id = USB_CMD_WRITE,
	.group= USB_CMD_GROUP_NAME,
	.desc = "write one usb device\n"
			"usage: usb write [module] [addr] [value]\n"
			"example:\n"
			"    usb write ana 0x60 0x0001  - write analog [60] = 0x0001\n"
			"    usb write pmu 0x03 0x0002  - write pmu [03] = 0x0001\n"
			"    usb write dig 0x40300060 0x1234abcd - write digital [0x40300060] = 0x1234abcd\n"
			"    usb write mem 0x20000000 0x1234abcd - write memory [0x20000000] = 0x1234abcd\n"
			"    usb write rst    - reset system\n"
			"    usb write rstcdc - reset system with USB CDC device\n"
			"\n",
	.cdata = &cdata_usb,
	.priv  = &usbctrl,
	.handler = usb_cmd_handler,
};

static struct comm_cmd usb_cmd_test = {
	.id = USB_CMD_TEST,
	.group= USB_CMD_GROUP_NAME,
	.desc = "test usb device\n"
			"usage: usb test\n"
			"\n",
	.cdata = &cdata_usb,
	.priv  = &usbctrl,
	.handler = usb_cmd_handler,
};

static struct comm_cmd usb_cmd_fm = {
	.id = USB_CMD_FM,
	.group= USB_CMD_GROUP_NAME,
	.desc = "fm device commands\n"
			"usage: usb fm [cmd] [arg]\n"
			"example: \n"
			"    usb fm on            - power on device\n"
			"    usb fm off           - power off device\n"
			"    usb fm band          - get band\n"
			"    usb fm band 0        - set band to 0\n"
			"    usb fm channel       - get channel\n"
			"    usb fm channel 10390 - set channel to 10390\n"
			"    usb fm volume        - get volume\n"
			"    usb fm volume 8      - set volume to 8\n"
			"    usb fm test          - test device\n"
			"\n",
	.cdata = &cdata_usb,
	.priv  = &usbctrl,
	.handler = usb_cmd_handler,
};


static struct comm_cmd *usb_cmds[] = {
	&usb_cmd_list,
	&usb_cmd_list1,
	&usb_cmd_bind,
	&usb_cmd_unbind,
	&usb_cmd_read,
	&usb_cmd_write,
	&usb_cmd_test,
	&usb_cmd_fm,
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
				USB_TRACE(0, "register cmd %s failed, %d\n",
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
				USB_TRACE(0, "unregister cmd %s failed, %d\n",
					cmd->name, r);
				break;
			}
		}
	}
	return r;
}
