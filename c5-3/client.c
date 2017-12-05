#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

static int send_message(int socket_fd, char *msg)
{
	char *end = "\r\n";
	int len = strlen(msg), r;

	r = write(socket_fd, msg, len);
	r = write(socket_fd, end, 2);
	return r;
}

int client_sock(int argc, char **argv)
{
	int fd, r, i;
	struct sockaddr_in sa;
	int port = 1234;
	char *ipstr = "127.0.0.1";

	fd = socket(PF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("socket failed\n");
		return -1;
	}
	printf("socket fd=%d\n", fd);

	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr(ipstr);
	sa.sin_port = htons(port);

	r = connect(fd, (const struct sockaddr *)&sa, sizeof(sa));
	if (r) {
		perror("connect failed\n");
		return -1;
	}
	printf("connet server ok, ip: %s, port: %d\n", ipstr, port);

	for(i = 1; i < argc; i++) {
		send_message(fd, argv[i]);
		sleep(1);
	}
	close(fd);
	printf("client exit\n");
	return 0;
}

int show_host_info(int argc, char **argv)
{
	char *host;
	struct hostent *hinfo;
	char myname[255];
	char **alias, **addrs;

	if (argc > 1) {
		host = argv[1];
	} else {
		gethostname(myname, 255);
		host = myname;
	}
	printf("host name: %s\n", host);

	hinfo = gethostbyname(host);
	if (!hinfo) {
		printf("get host info failed\n");
		return -1;
	}
	printf("hinfo, name: %s, addr type: %d, addr len: %d\n",
		hinfo->h_name, hinfo->h_addrtype, hinfo->h_length);

	alias = hinfo->h_aliases;
	while(*alias) {
		printf("hinfo, alias: %s\n", *alias);
		alias++;
	}

	printf("AF_INET=%d, PF_INET=%d\n", AF_INET, PF_INET);
	addrs = hinfo->h_addr_list;
	while(*addrs) {
		struct in_addr *pin = (struct in_addr *)*addrs;
		printf("hinfo, addr: %s\n", inet_ntoa(*pin));
		addrs++;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	show_host_info(argc, argv);
//	client_sock(argc, argv);
	return 0;
}
