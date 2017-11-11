#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>

int server(int client_socket)
{
	int disconnect, len, r;
	char *text;

	disconnect = 0;
	while (1) {
		r = read(client_socket, &len, sizeof(len));
		if (!r)
			break;

		text = (char *)malloc(len + 1);

		memset(text, 0, len + 1);

		r = read(client_socket, text, len);
		if (r)
			printf("got message: %s\n", text);

		if (!strcmp(text, "quit"))
			disconnect = 1;

		free(text);

		if (disconnect)
			break;
	}
	return disconnect;
}

int main(int argc, char *argv[])
{
	char *socket_name;
	int socket_fd, quit, r;
	struct sockaddr_un name;

	if (argc < 2) {
		printf("server 's name ?\n");
		return -1;
	}
	socket_name = argv[1];

	printf("server %s is waitting connection ...\n", socket_name);

	socket_fd = socket(PF_LOCAL, SOCK_STREAM, 0);
	if (socket_fd < 0) {
		printf("create socket failed %d\n", errno);
		return -1;
	}
	name.sun_family = AF_LOCAL;
	strcpy(name.sun_path, socket_name);

	r = bind(socket_fd, (const struct sockaddr *)&name, SUN_LEN(&name));
	if (r) {
		printf("bind socket failed %d\n", errno);
		return -1;
	}

	r = listen(socket_fd, 5);
	if (r) {
		printf("listen socket failed %d\n", errno);
		return -1;
	}

	do {
		struct sockaddr_un client_name;
		socklen_t client_name_len;
		int client_socket_fd;

		client_socket_fd = accept(socket_fd,
			(struct sockaddr *)&client_name, &client_name_len);

		quit = server(client_socket_fd);

		close(client_socket_fd);
	} while(!quit);

	close(socket_fd);
	unlink(socket_name);

	printf("server %s disconnected\n", socket_name);

	return 0;
}
