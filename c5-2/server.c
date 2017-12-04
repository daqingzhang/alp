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
	while (!disconnect) {
		r = read(client_socket, &len, sizeof(len));
		if (r <= 0)
			break;

		if (!len)
			break;

		ps = (char *)malloc(len + 1);
		memset(ps, 0, len + 1);

		r = read(client_socket, ps, len);
		if (r <= 0) {
			printf("read error %d\n", r);
			free(ps);
			break;
		}
		printf("get msg:%s\n", ps);

		if (!strcmp(ps, "quit"))
			disconnect = 1;
		free(ps);
	}
	return disconnect;
}

static int server_socket(const char *name)
{
	int fd, quit, r;
	struct sockaddr_un sa;

	fd = socket(PF_LOCAL, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("get socket fd failed\n");
		return -1;
	}
	printf("fd=%d\n", fd);

	sa.sun_family = AF_LOCAL;
	strcpy(sa.sun_path, name);

	r = bind(fd, (const struct sockaddr *)&sa, sizeof(struct sockaddr));
	if (r) {
		perror("bind socket failed\n");
		return -1;
	}
	printf("bind %s okay\n", name);

	r = listen(fd, 5);
	if (r) {
		perror("listen socket failed\n");
		return -1;
	}

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
		printf("client fd=%d\n", cli_fd);

		quit = server_run(cli_fd);
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
	char *name;

	if (argc < 2) {
		printf("./server [name]\n");
		return -1;
	}
	name = argv[1];
	server_socket(name);

	return 0;
}
