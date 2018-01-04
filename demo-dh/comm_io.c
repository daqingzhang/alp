#include <comm_cmd.h>
#include <sock_cmd.h>

/*
 * comm_fgetc() should be blocked until data come
 */

int comm_fgetc(void)
{
	return sock_getc_blocked();
}

int comm_puts(const char *s)
{
	return sock_write(s, strlen(s));
}

int user_cmd_register(void)
{
	int r = 0;

	r = sock_cmd_register();
	return r;
}

int user_cmd_unregister(void)
{
	int r = 0;

	r = sock_cmd_unregister();
	return r;
}
