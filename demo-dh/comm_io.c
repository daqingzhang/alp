#include <string.h>
#include <comm_cmd.h>
#include <server.h>

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
