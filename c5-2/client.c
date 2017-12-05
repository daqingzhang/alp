#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>

#include <netinet/in.h>
#include <arpa/inet.h>

static int send_message(int socket_fd, char *msg)
{
	int len = strlen(msg), r;

	r = write(socket_fd, &len, sizeof(len));
	r += write(socket_fd, msg, len);
	return r;
}

int client_socket(const char *name, int argc, char **argv)
{
	int fd, r, i, len;
	struct sockaddr_un sa;

	fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("socket failed\n");
		return -1;
	}
	printf("socket fd=%d\n", fd);

	sa.sun_family = AF_UNIX;
	strcpy(sa.sun_path, name);

	r = connect(fd, (const struct sockaddr *)&sa,
				sizeof(struct sockaddr));
	if (r) {
		perror("connect failed\n");
		return -1;
	}

	for(i = 2; i < argc; i++) {
		len = send_message(fd, argv[i]);
		if (len < 1) {
			printf("semd message[%d] failed\n", i);
			break;
		}
		sleep(1);
	}
	close(fd);
	printf("client exit\n");
	return 0;
}

int client_socket2(int argc, char **argv)
{
	int fd, r, i, len;
	struct sockaddr_in sa;

	fd = socket(PF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("socket failed\n");
		return -1;
	}
	printf("socket fd=%d\n", fd);

	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr("127.0.0.1");
	sa.sin_port = htons(1234);

	r = connect(fd, (const struct sockaddr *)&sa,
				sizeof(struct sockaddr));
	if (r) {
		perror("connect failed\n");
		return -1;
	}

	for(i = 2; i < argc; i++) {
		len = send_message(fd, argv[i]);
		if (len < 1) {
			printf("semd message[%d] failed\n", i);
			break;
		}
		sleep(1);
	}
	close(fd);
	printf("client exit\n");
	return 0;
}

int main(int argc, char *argv[])
{
#if 1
	char *name;
	if (argc < 3) {
		printf("./client [name] [message]\n");
		return -1;
	}
	name = argv[1];

	client_socket(name, argc, argv);
#else
	client_socket2(argc, argv);
#endif
	return 0;
}
