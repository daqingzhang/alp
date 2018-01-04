#include <sock_cmd.h>
#include <comm_cmd.h>

static struct comm_data cdata_sock;
static void sock_cmd_handler(int id, void *cdata, void *priv);

static struct comm_cmd cmd_sock = {
	.id = 1,
	.name = "sock",
	.desc = "socket command\n"
		"usage:\n"
		"sock echo [string]	- echo string to socket\n"
		"\n",

	.cdata = &cdata_sock,
	.priv = NULL,
	.handler = sock_cmd_handler,
};

static int do_cmd_sock_echo(int argc, char *argv[])
{
	char *ps;

	DBG("%s\n", __func__);

	ps = argv[2];

	comm_puts("\n");
	comm_puts(ps);
	comm_puts("\n");
	return 0;
}

static void sock_cmd_handler(int id, void *cdata, void *priv)
{
	int r;
	char *key, *cmd;
	struct comm_data *pcd = cdata;

	if (!pcd) {
		ioprintf("null cdata\n");
		return;
	}
	if (pcd->argc < 2) {
		ioprintf("%s\n", cmd_sock.desc);
		return;
	}
	DBG("id=%d, cdata(%p)\n", id, pcd);

	key = pcd->argv[0];
	cmd = pcd->argv[1];
	if (!strcmp(cmd, "echo")) {
		r = do_cmd_sock_echo(pcd->argc, pcd->argv);
	} else if (!strcmp(cmd, "write")) {
	} else if (!strcmp(cmd, "read")) {
	} else {
	}

	if (r)
		ioprintf("cmd %s id=%d error %d\n", key, id, r);

	comm_data_init(pcd);
}

int sock_cmd_register(void)
{
	comm_data_init(cmd_sock.cdata);
	return comm_cmd_register(&cmd_sock);
}

int sock_cmd_unregister(void)
{
	return comm_cmd_unregister(&cmd_sock);
}
