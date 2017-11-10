#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>

int write_text(int socket_fd, char *text)
{
	int len = strlen(text) + 1;
	int r;

	r = write(socket_fd, &len, sizeof(len));

	r += write(socket_fd, text, len);

	return r;
}

int main(int argc, char *argv[])
{
	char *socket_name;
	char *message;
	int socket_fd;
	struct sockaddr_un name;
	int cnt = 0xffff;

	if (argc < 3) {
		printf("./%s name message\n", __func__);
		return -1;
	}

	socket_name = argv[1];
	message = argv[2];

	socket_fd = socket(PF_LOCAL, SOCK_STREAM, 0);

	name.sun_family = AF_LOCAL;
	strcpy(name.sun_path, socket_name);

	connect(socket_fd, (const struct sockaddr *)&name, SUN_LEN(&name));

	while (1) {
		write_text(socket_fd, message);
		sleep(1);
		if (!cnt)
			break;
		cnt--;
	}

	close(socket_fd);
	return 0;
}

