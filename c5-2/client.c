#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>

int write_text(int socket_fd, char *text)
{
	int len = strlen(text);
	int r;

	r = write(socket_fd, &len, sizeof(len));

	r += write(socket_fd, text, len);

	return r;
}

int main(int argc, char *argv[])
{
	char *socket_name;
	char *message;
	int socket_fd, i;
	struct sockaddr_un name;

	if (argc < 3) {
		printf("client name and message ?\n");
		return -1;
	}
	socket_name = argv[1];

	printf("client connects server %s, send %d messages ...\n",
		socket_name, argc - 2);

	socket_fd = socket(PF_LOCAL, SOCK_STREAM, 0);

	name.sun_family = AF_LOCAL;
	strcpy(name.sun_path, socket_name);

	connect(socket_fd, (const struct sockaddr *)&name, SUN_LEN(&name));

	for(i = 2;i < argc; i++) {
		message = argv[i];
		write_text(socket_fd, message);
		sleep(1);
	}
	close(socket_fd);
	printf("client disconnect server %s\n", socket_name);
	return 0;
}
