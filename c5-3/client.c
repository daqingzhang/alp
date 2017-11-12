#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
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

#define BUF_SIZE 10000
static char buffer[BUF_SIZE];
int get_home_page(int socket_fd)
{
	int len = 0;
	ssize_t num;

	sprintf(buffer, "GET /\n");

	len = write(socket_fd, buffer, strlen(buffer));

	while (1) {
		num = read(socket_fd, buffer, BUF_SIZE);
		if (num == 0)
			return 0;
		printf("num = %d\n", num);
		fwrite(buffer, sizeof(char), num, stdout);
	}
	return len;
}

int main(int argc, char *argv[])
{
	int socket_fd, r;
	struct sockaddr_in name;
	struct hostent *hostinfo;

	if (argc < 2) {
		printf("host name ?\n");
		return -1;
	}

	socket_fd = socket(PF_INET, SOCK_STREAM, 0);

	name.sin_family = AF_INET;

	hostinfo = gethostbyname(argv[1]);
	if(hostinfo == NULL)
		return -1;
	else
		name.sin_addr = *((struct in_addr *)hostinfo->h_addr);

	name.sin_port = htons(80);

	r = connect(socket_fd, (const struct sockaddr *)&name,
		sizeof(struct sockaddr_in));

	if (r < 0) {
		printf("connect failed\n");
		return -1;
	}
	get_home_page(socket_fd);

	close(socket_fd);
	printf("disconnect server\n");
	return 0;
}
