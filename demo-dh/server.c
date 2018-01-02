#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <oslib.h>
#include <comm_cmd.h>
#include <server.h>
#include <serial_thread.h>

#define SERVER_BUF_SIZE 4096

struct sock_data {
	int server_fd;
	int client_fd;
	int port;
	int ql;
	int ipv;
	char *ips;
	struct sockaddr_in server_sa;
	struct sockaddr_in client_sa;
	socklen_t client_len;

	char temp[SERVER_BUF_SIZE];
	int tmplen;
	int tmpsize;
};

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
		DBG("%s, error %d\n", __func__, cnt);
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
		if (cnt == 0)
			usleep(20);
		else if (cnt < 0)
			DBG("%s, error %d\n", __func__, cnt);
		else
			break;
	}
	DBG("%s, temp=%2x\n", __func__, temp);
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
		DBG("%s, error %d\n", __func__, cnt);
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
		DBG("%s, error %d, len=%d\n", __func__, cnt, len);
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
			DBG("%s, error %d, len=%d\n", __func__, cnt, len);
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
		DBG("%s, error %d\n", __func__, cnt);
	return cnt;
}

static int server_run(int argc, char *argv[])
{
	return comm_main(argc, argv);
}

static int server_start(int argc, char *argv[])
{
	int fd, r;
	struct sock_data *psd = &sockdata;

	fd = socket(PF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("get socket fd failed\n");
		return -1;
	}
	psd->server_fd = fd;

	psd->server_sa.sin_family = AF_INET;
	psd->server_sa.sin_port = htons(psd->port);
	psd->server_sa.sin_addr.s_addr = inet_addr(psd->ips);
	r = bind(psd->server_fd, (const struct sockaddr *)&psd->server_sa,
			sizeof(psd->server_sa));
	if (r) {
		perror("bind socket failed\n");
		return -1;
	}
	DBG("bind ok, fd:%d, ip:%s, port:%d\n",
		psd->server_fd, psd->ips, psd->port);

	r = listen(psd->server_fd, psd->ql);
	if (r) {
		perror("listen socket failed\n");
		return -1;
	}

	DBG("wait connection ...\n");
	fd = accept(psd->server_fd, (struct sockaddr *)&psd->client_sa,
			&psd->client_len);
	if (fd < 0)
		perror("client fd error\n");

	psd->client_fd = fd;
	DBG("client %d connceted\n", psd->client_fd);

	server_run(argc, argv);

//	close(psd->client_fd);
//	DBG("client closed\n");

	close(psd->server_fd);
	printf("server closed\n");
	return 0;
}

int main(int argc, char *argv[])
{
	return server_start(argc, argv);
}
