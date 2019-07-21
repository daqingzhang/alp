#include <unistd.h>
#include <debug.h>
#include <sock_cmd.h>

static struct sock_data sockdata = {
	.server_fd = -1,
	.client_fd = -1,
	.port = 1234,
	.ipv = 0,
	.ips = "127.0.0.1",
	.ql = 5,
	.client_len = 0,
	.tmplen = 0,
	.tmpsize = SERVER_BUF_SIZE,
};

struct sock_data *sock_get_sockdata(void)
{
	return &sockdata;
}

/*
 * sock_getc - get one char from socket port
 * return: data or 0(failed)
 */

int sock_getc(void)
{
	int cnt;
	char temp = 0;
	struct sock_data *psd = &sockdata;

	cnt = read(psd->client_fd, &temp, 1);
	if (cnt < 0)
		SOCK_TRACE(0, "%s, error %d\n", __func__, cnt);
	return temp;
}

/*
 * sock_getc_blocked - get one char from socket port
 * It will be blocked until required data is read.
 * return: data or 0(failed)
 */

int sock_getc_blocked(void)
{
	int cnt;
	char temp = 0;
	struct sock_data *psd = &sockdata;

	while(1) {
		cnt = read(psd->client_fd, &temp, 1);
		if (cnt == 0) {
			usleep(20);
		} else if (cnt < 0) {
			SOCK_TRACE(0, "%s, error %d\n", __func__, cnt);
		} else {
			break;
		}
	}
	SOCK_TRACE(1, "%s, temp=%2x\n", __func__, temp);
	return temp;
}

/*
 * sock_putc - put one char to socket port
 * return: data
 */

int sock_putc(char c)
{
	struct sock_data *psd = &sockdata;
	int cnt;

	cnt = write(psd->client_fd, &c, 1);
	if (cnt != 1)
		SOCK_TRACE(0, "%s, error %d\n", __func__, cnt);
	return c;
}

/*
 * sock_read - read n chars from socket port
 * param:
 *	buf	point to a buffer of data
 *	len	the number of data
 * return: the number of data read from
 */

int sock_read(char *buf, int len)
{
	struct sock_data *psd = &sockdata;
	int cnt;

	cnt = read(psd->client_fd, buf, len);
	if (cnt != len)
		SOCK_TRACE(0, "%s, error %d, len=%d\n", __func__, cnt, len);
	return cnt;
}

/*
 * sock_read_blocked - read n chars from socket port
 * It will be blocked until required data is read.
 * param:
 *	buf	point to a buffer of data
 *	len	the number of data
 * return: the number of data read from
 */

int sock_read_blocked(char *buf, int len)
{
	struct sock_data *psd = &sockdata;
	int cnt, last;

	last = len;
	while(1) {
		if (last == 0)
			break;
		cnt = read(psd->client_fd, buf, last);
		if (cnt > 0) {
			last -= cnt;
			buf += cnt;
		}
		if (cnt < 0) {
			SOCK_TRACE(0, "%s, error %d, len=%d\n", __func__, cnt, len);
			break;
		}
		usleep(20);
	}
	return (len - last);
}

/*
 * sock_write - write n chars to socket port
 * param:
 *	buf	point to a buffer of data
 *	len	the number of data
 * return: the number of data written to
 */

int sock_write(const char *buf, int len)
{
	struct sock_data *psd = &sockdata;
	int cnt;

	cnt = write(psd->client_fd, buf, len);
	if (cnt != len)
		SOCK_TRACE(0, "%s, error %d\n", __func__, cnt);
	return cnt;
}
