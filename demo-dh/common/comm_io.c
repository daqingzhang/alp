#include <comm_cmd.h>
#include <sock_cmd.h>

/*
 * comm_fgetc() should be blocked until data come
 */

typedef int (*cmd_handle_t)(void);

struct cmd_hook {
	cmd_handle_t init;
	cmd_handle_t exit;
};

int usb_cmd_register(void);
int usb_cmd_unregister(void);
int sock_cmd_unregister(void);
int sock_cmd_unregister(void);

static struct cmd_hook cmd_hooks[] = {
	{
		.init = usb_cmd_register,
		.exit = usb_cmd_unregister,
	},
	{
		.init = sock_cmd_register,
		.exit = sock_cmd_unregister,
	}
};

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

int user_cmd_register(void)
{
	int r = 0;
	int i;
	struct cmd_hook *h;

	for(i = 0; i < ARRAY_SIZE(cmd_hooks); i++) {
		h = &cmd_hooks[i];
		if (h->init) {
			r = h->init();
			if (r) {
				break;;
			}
		}
	}
	return r;
}

int user_cmd_unregister(void)
{
	int r = 0;
	int i;
	struct cmd_hook *h;

	for(i = 0; i < ARRAY_SIZE(cmd_hooks); i++) {
		h = &cmd_hooks[i];
		if (h->exit) {
			r = h->exit();
			if (r) {
				break;;
			}
		}
	}
	return r;
}
