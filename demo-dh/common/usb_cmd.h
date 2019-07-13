#ifndef __USB_CMD_H__
#define __USB_CMD_H__

enum {
	USB_IDX_GROUP = 0,
	USB_IDX_CMD   = 1,
	USB_IDX_ARGC1 = 2,
	USB_IDX_ARGC2 = 3,
	USB_IDX_ARGC3 = 4,
};

enum usb_cmd {
	USB_CMD_LIST,
	USB_CMD_BIND,
	USB_CMD_UNBIND,
	USB_CMD_READ,
	USB_CMD_WRITE,
	USB_CMD_TEST,
};

enum {
	USB_MOD_NULL,
	USB_MOD_ANA,
	USB_MOD_PMU,
	USB_MOD_DIG,
	USB_MOD_MEM,
	USB_CMD_EXEC,
	USB_CMD_RESET,
	USB_CMD_RESET_CDC,
};

struct usb_cmd_t {
	char *name;
	int id;
};

int usb_cmd_register(void);
int usb_cmd_unregister(void);

#endif
