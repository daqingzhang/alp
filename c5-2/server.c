#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>

int server(int client_socket)
{
	while (1) {
		int length, r, cnt=0;
		char *text;

		r = read(client_socket, &length, sizeof(length));
		if (!r)
			return 0; // client closed connection

		text = (char *)malloc(length);

		memset(text, 0, length);

		r = read(client_socket, text, length);
		if (r)
			printf("%s , cnt: %d\n", text, ++cnt);

		free(text);

		if (!strcmp(text, "quit"))
			return 1;
	}
}

int main(int argc, char *argv[])
{
	char *socket_name;
	int socket_fd;
	struct sockaddr_un name;
	int quit, r;

	if (argc < 2) {
		printf("socket name not find\n");
		return -1;
	}
	socket_name = argv[1];

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

	printf("server connection quit\n");

	close(socket_fd);
	unlink(socket_name);

	return 0;
}
