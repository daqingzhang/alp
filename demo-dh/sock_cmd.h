#ifndef __SOCK_CMD_H__
#define __SOCK_CMD_H__
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_BUF_SIZE 4096

/*
 * socket data structure
 ***************************************************
 */
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

#define SOCK_PDATA_BUF_SIZE 1024
struct sock_pdata {
	int size;
	int cnt;
	unsigned int buf[SOCK_PDATA_BUF_SIZE];
};

/*
 * socket IO function
 ***************************************************
 */

int sock_getc(void);
int sock_getc_blocked(void);
int sock_putc(char c);
int sock_read(char *buf, int len);
int sock_read_blocked(char *buf, int len);
int sock_write(const char *buf, int len);
struct sock_data *sock_get_sockdata(void);

/*
 * socket command function
 ***************************************************
 */
int sock_cmd_register(void);
int sock_cmd_unregister(void);

#endif
