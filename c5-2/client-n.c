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

static int client_socket(const char *name, int argc, char **argv)
{
	int fd, r, i;
	struct sockaddr_un sa;

	fd = socket(PF_LOCAL, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("socket failed\n");
		return -1;
	}
	printf("fd=%d\n", fd);

	sa.sun_family = AF_LOCAL;
	strcpy(sa.sun_path, name);

	r = connect(fd, (const struct sockaddr *)&sa,
				sizeof(struct sockaddr));
	if (r) {
		perror("connect failed\n");
		return -1;
	}

	for(i = 2; i < argc; i++) {
		write_text(fd, argv[i]);
		sleep(1);
	}
	close(fd);
	printf("client exit\n");

	return 0;
}

int main(int argc, char *argv[])
{
	char *sock_name;

	if (argc < 3) {
		printf("client name and message ?\n");
		return -1;
	}
	sock_name = argv[1];

	client_socket(sock_name, argc, argv);

	return 0;
}
