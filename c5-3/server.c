#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>

#include <netinet/in.h>
#include <arpa/inet.h>

static int server_run(int client_fd)
{
#define SERVER_BUF_SIZE 1024
	int quit = 0;
	int cnt, len, size;
	char *ps;
	char *end = "\r\n";
	char *start = ">";

	ps = (char *)malloc(SERVER_BUF_SIZE);
	size = SERVER_BUF_SIZE;

	do {
		memset(ps, 0x0, size);

		len = 0;
		while (1) {
			unsigned char t;

			cnt = read(client_fd, &t, 1);
			if (cnt < 0) {
				printf("read error, exit\n");
				free(ps);
				exit(-1);
				break;
			} else if (cnt > 0) {
				printf("get char: %2x\n", t);
				if (t == '\n') {
					len--;
					ps[len] = '\0';
					break;
				}
				ps[len] = t;
				len++;
			} else {
				usleep(20);
			}
		}
		if (len) {
			printf("msg:%s\n", ps);
#if 1
			cnt =write(client_fd, start, 2);
			cnt += write(client_fd, ps, len);
			cnt +=write(client_fd, end, 2);
			printf("write %d bytes\n", cnt);
#endif
			if (!strcmp(ps, "quit"))
				quit = 1;
		//	break;
		}
	} while(!quit);

	free(ps);
	return quit;
}

int server_sock(void)
{
	int fd, quit, r;
	struct sockaddr_in sa;
	int port = 1234;
	char *ipstr = "127.0.0.1";

	fd = socket(PF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("get socket fd failed\n");
		return -1;
	}
	printf("fd=%d\n", fd);

	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	sa.sin_addr.s_addr = inet_addr(ipstr);

	r = bind(fd, (const struct sockaddr *)&sa, sizeof(sa));
	if (r) {
		perror("bind socket failed\n");
		return -1;
	}
	printf("bind ok, ip: %s, port: %d\n", ipstr, port);

	r = listen(fd, 5);
	if (r) {
		perror("listen socket failed\n");
		return -1;
	}

	do {
		int cli_fd;
		struct sockaddr_in cli_sa;
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
	printf("server closed\n");

	return 0;
}

int main(int argc, char *argv[])
{
	server_sock();
	return 0;
}
