#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <comm_cmd.h>
#include <sock_cmd.h>
#include <serial_thread.h>
#ifdef TEST
#include <test.h>
#endif

int server_run(int argc, char *argv[])
{
	return comm_main(argc, argv);
}

int server_main(int argc, char *argv[])
{
	int fd, r;
	struct sock_data *psd = sock_get_sockdata();

	fd = socket(PF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("get socket fd failed\n");
		return -1;
	}
	psd->server_fd = fd;

	psd->server_sa.sin_family = AF_INET;
	psd->server_sa.sin_port = htons(psd->port);
	psd->server_sa.sin_addr.s_addr = inet_addr(psd->ips);
	r = bind(psd->server_fd, (const struct sockaddr *)&psd->server_sa,
			sizeof(psd->server_sa));
	if (r) {
		perror("bind socket failed\n");
		goto exit;
	}
	SOCK_TRACE(2, "bind ok, fd:%d, ip:%s, port:%d\n",
		psd->server_fd, psd->ips, psd->port);

	r = listen(psd->server_fd, psd->ql);
	if (r) {
		perror("listen socket failed\n");
		goto exit;
	}

	SOCK_TRACE(2, "wait connection ...\n");
	fd = accept(psd->server_fd, (struct sockaddr *)&psd->client_sa,
			&psd->client_len);
	if (fd < 0)
		perror("client fd error\n");

	psd->client_fd = fd;
	SOCK_TRACE(2, "client %d connceted\n", psd->client_fd);

	server_run(argc, argv);

//	close(psd->client_fd);
//	SOCK_TRACE("client closed\n");
exit:
	unlink(psd->ips);
	close(psd->server_fd);
	SOCK_TRACE(2, "server closed\n");
	return 0;
}

int main(int argc, char *argv[])
{
#ifdef USE_SOCKET
	return server_main(argc, argv);
#elif defined(TEST)
	return test_main(argc, argv);
#else
	return comm_main(argc, argv);
#endif
}
