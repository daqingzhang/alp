#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>

static int server_run(int client_socket)
{
	int disconnect, len, r;
	char *ps;

	disconnect = 0;
	while (1) {
		r = read(client_socket, &len, sizeof(len));
		if (!r)
			break;

		if (!len)
			break;

		ps = (char *)malloc(len + 1);

		memset(ps, 0, len + 1);

		r = read(client_socket, ps, len);
		if (r)
			printf("get msg:%s\n", ps);

		if (!strcmp(ps, "quit"))
			disconnect = 1;

		free(ps);
	}
	return disconnect;
}

typedef int (*sock_handler_t)(int fd);

static int server_socket(const char *name, sock_handler_t handler)
{
	int fd, quit, r;
	struct sockaddr_un sa;

	fd = socket(PF_LOCAL, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("socket failed\n");
		return -1;
	}
	printf("socket fd is %d\n", fd);

	sa.sun_family = AF_LOCAL;
	strcpy(sa.sun_path, name);

	r = bind(fd, (const struct sockaddr *)&sa,
			sizeof(struct sockaddr));
	if (r) {
		perror("bind failed\n");
		return -1;
	}
	printf("bind %s okay\n", name);

	r = listen(fd, 5);
	if (r) {
		perror("listen failed\n");
		return -1;
	}

	quit = 0;
	do {
		int cli_fd;
		struct sockaddr_un cli_sa;
		socklen_t cli_len;

		printf("wait client connection ...\n");
		cli_fd = accept(fd, (struct sockaddr *)&cli_sa, &cli_len);
		if (cli_fd < 0) {
			perror("client fd error\n");
			continue;
		}
		printf("client fd is %d\n", cli_fd);

		quit = handler(cli_fd);
		printf("server exit\n");

		close(cli_fd);
	} while(!quit);

	close(fd);
	unlink(name);
	printf("server closed\n");

	return 0;
}

int main(int argc, char *argv[])
{
	char *ser_name;

	if (argc < 2) {
		printf("server 's name ?\n");
		return -1;
	}
	ser_name = argv[1];
	server_socket(ser_name, server_run);

	return 0;
}
