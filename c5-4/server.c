#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>

//#define DEBUG

#ifdef DEBUG
#define DBG printf
#else
#define DBG(...) do{}while(0)
#endif

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

int server_accept(int fd)
{
	int q = 0;
	int cli_fd;
	struct sockaddr_in cli_sa;
	socklen_t cli_len;

	printf("%s, fd=%d\n", __func__, fd);

	do {
		printf("wait client connection ...\n");
		cli_fd = accept(fd, (struct sockaddr *)&cli_sa, &cli_len);
		if (cli_fd < 0) {
			perror("client fd error\n");
			continue;
		}
		printf("client fd=%d\n", cli_fd);
		q = server_run(cli_fd);
	} while (!q);

	printf("server exit\n");
	close(cli_fd);
//	close(fd);

	return 0;
}

struct server_info {
	int fd;
	int port;
	unsigned int ip;
	struct sockaddr_in sa;
	fd_set fds;
};

struct server_info ser_info[2] = {
	{
		.port = 1234,
		.ip = INADDR_ANY,
	},
	{
		.port = 3333,
		.ip = INADDR_ANY,
	},
};

int server_setup(struct server_info *info)
{
	int fd, r;
	struct sockaddr_in *paddr = &info->sa;
	fd_set *pfds = &info->fds;

	fd = socket(PF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("get socket fd failed\n");
		return -1;
	}
	info->fd = fd;

	paddr->sin_family = AF_INET;
	paddr->sin_port = htons(info->port);
	paddr->sin_addr.s_addr = htonl(info->ip);//any connects is okay
	r = bind(fd, (const struct sockaddr *)paddr, sizeof(*paddr));
	if (r) {
		perror("bind socket failed\n");
		return -1;
	}

	r = listen(fd, 5);
	if (r) {
		perror("listen socket failed\n");
		return -1;
	}

	FD_ZERO(pfds);
	FD_SET(fd, pfds);

	printf("setup server: fd=%d, ip=%x, port=%d\n",
		info->fd, info->ip, info->port);
	return 0;
}

int server_scan(void)
{
	int r, id = 0;
	struct server_info *info = ser_info;

	r = server_setup(&info[0]);
	r += server_setup(&info[1]);
	if (r) {
		perror("setup server failed\n");
		return -1;
	}

	while(1) {
		int tmp_fd, ser_fd;
		fd_set tmp_fds;
		struct timeval tim;

		tim.tv_sec = 1;
		tim.tv_usec = 0;

		tmp_fds = info[id].fds;
		ser_fd  = info[id].fd;

		DBG("server waiting\n");
		r = select(FD_SETSIZE, &tmp_fds, 0, 0, &tim);
		if (r < 0) {
			perror("select error\n");
			break;
		}
		if (r == 0) {
			DBG("select timeout\n");
			id = 1 - id;
			continue;
		}

		if (FD_ISSET(ser_fd, &tmp_fds)) {
			server_accept(ser_fd);
		} else {
			for(tmp_fd = 0;tmp_fd < FD_SETSIZE; tmp_fd++)
				if (FD_ISSET(tmp_fd, &tmp_fds))
					printf("another fd %d is active\n", tmp_fd);
		}
	}
	close(info[0].fd);
	close(info[1].fd);
	return 0;
}

int main(int argc, char *argv[])
{
	server_scan();
	return 0;
}
