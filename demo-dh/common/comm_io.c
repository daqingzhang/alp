#include <comm_cmd.h>

#ifdef CMD_MODULE_USB
int usb_cmd_register(void);
int usb_cmd_unregister(void);
#endif

#ifdef CMD_MODULE_SOCK
int sock_cmd_register(void);
int sock_cmd_unregister(void);
#endif

struct comm_cmd_module comm_cmd_modules[] = {
#ifdef CMD_MODULE_USB
	CMD_MODULE(usb_cmd_register, usb_cmd_unregister),
#endif

#ifdef CMD_MODULE_SOCK
	CMD_MODULE(sock_cmd_register, sock_cmd_unregister),
#endif
};

/*
 * comm_fgetc() should be blocked until data come
 */

int comm_fgetc(void)
{
#ifdef USE_SOCKET
	return sock_getc_blocked();
#else
	return fgetc(stdin);
#endif
}

int comm_puts(const char *s)
{
#ifdef USB_SOCKET
	return sock_write(s, strlen(s));
#else
	return printf("%s", s);
#endif
}

int comm_user_cmd_register(void)
{
	int r = 0;
	int i;
	struct comm_cmd_module *h;

	for(i = 0; i < ARRAY_SIZE(comm_cmd_modules); i++) {
		h = &comm_cmd_modules[i];
		if (h->init) {
			r = h->init();
			if (r) {
				break;;
			}
		}
	}
	return r;
}

int comm_user_cmd_unregister(void)
{
	int r = 0;
	int i;
	struct comm_cmd_module *h;

	for(i = 0; i < ARRAY_SIZE(comm_cmd_modules); i++) {
		h = &comm_cmd_modules[i];
		if (h->exit) {
			r = h->exit();
			if (r) {
				break;;
			}
		}
	}
	return r;
}
