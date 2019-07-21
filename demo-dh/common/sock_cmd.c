#include <sock_cmd.h>
#include <comm_cmd.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof((a)) / sizeof((a)[0]))
#endif

enum SOCK_CMD_ID {
	SOCK_CMD_ZERO,
	SOCK_CMD_ECHO,
	SOCK_CMD_MEM_D32,
	SOCK_CMD_MEM_W32,
	SOCK_CMD_MEM_R32,
	SOCK_CMD_REGS,
	SOCK_CMD_REG_W32,
	SOCK_CMD_REG_R32,
	SOCK_CMD_CPU_HALT,
	SOCK_CMD_CPU_RUN,
	SOCK_CMD_CPU_STEP,
	SOCK_CMD_CPU_STATE,
	SOCK_CMD_DL_XMODEM,
};

static void sock_data_init(void *priv)
{
	struct sock_pdata *ppd = priv;

	if (ppd) {
		ppd->cnt = 0;
		ppd->size = 0;
	}
}

static int do_cmd_echo(int argc, char *argv[])
{
	char *ps;

	SOCK_TRACE(1, "%s\n", __func__);

	ps = argv[1];

	comm_puts("\n");
	comm_puts(ps);
	comm_puts("\n");
	return 0;
}

static int do_cmd_md(int argc, char *argv[], int size)
{
	SOCK_TRACE(1, "%s\n", __func__);
	return 0;
}

static int do_cmd_mw(int argc, char *argv[], int size)
{
	SOCK_TRACE(1, "%s\n", __func__);
	return 0;
}

static int do_cmd_mr(int argc, char *argv[], int size)
{
	SOCK_TRACE(1, "%s\n", __func__);
	return 0;
}

static void sock_cmd_handler(int id, void *cdata, void *priv)
{
	int r;
	struct comm_data *pcd = cdata;
	struct sock_pdata *ppd = priv;

	switch(id) {
	case SOCK_CMD_ECHO:
		// echo test
		r = do_cmd_echo(pcd->argc, pcd->argv);
		break;
	case SOCK_CMD_MEM_D32:
		// memory display word
		r = do_cmd_md(pcd->argc, pcd->argv, 4);
		break;
	case SOCK_CMD_MEM_W32:
		// memory write word
		r = do_cmd_mw(pcd->argc, pcd->argv, 4);
		break;
	case SOCK_CMD_MEM_R32:
		// memory read word
		r = do_cmd_mr(pcd->argc, pcd->argv, 4);
		break;
	case SOCK_CMD_REGS:
	case SOCK_CMD_REG_W32:
	case SOCK_CMD_REG_R32:
	case SOCK_CMD_CPU_HALT:
	case SOCK_CMD_CPU_RUN:
	case SOCK_CMD_CPU_STEP:
	case SOCK_CMD_CPU_STATE:
	case SOCK_CMD_ZERO:
	case SOCK_CMD_DL_XMODEM:
	default:
		break;
	}
	if (r)
		SOCK_TRACE(0, "exec cmd %d error %d\n", id, r);

	comm_data_init(pcd);
	sock_data_init(ppd);
}

static struct sock_pdata spdata;
static struct comm_data cdata_sock;

static struct comm_cmd cmd_echo = {
	.id = SOCK_CMD_ECHO,
	.name = "echo",
	.desc = "echo string\n"
		"echo [string]		- echo string to socket\n"
		"\n",
	.cdata = &cdata_sock,
	.priv = &spdata,
	.handler = sock_cmd_handler,
	.nogroup = 1,
};

static struct comm_cmd cmd_md32 = {
	.id = SOCK_CMD_MEM_D32,
	.name = "md32",
	.desc = "display memory word\n"
		"md32 [addr]		- display memory word at [addr]\n"
		"md32 [addr] [n]	- display memory n words at [addr]\n"
		"\n",
	.cdata = &cdata_sock,
	.priv = &spdata,
	.handler = sock_cmd_handler,
	.nogroup = 1,
};

static struct comm_cmd cmd_mw32 = {
	.id = SOCK_CMD_MEM_W32,
	.name = "mw32",
	.desc = "write word to memory\n"
		"mw32 [addr] [val]	- write [val] to memory [addr]\n"
		"\n",
	.cdata = &cdata_sock,
	.priv = &spdata,
	.handler = sock_cmd_handler,
	.nogroup = 1,
};

static struct comm_cmd cmd_mr32 = {
	.id = SOCK_CMD_MEM_R32,
	.name = "mr32",
	.desc = "read word from memory\n"
		"mr32 [addr] [n]	- read [val] to memory [addr]\n"
		"\n",
	.cdata = &cdata_sock,
	.priv = &spdata,
	.handler = sock_cmd_handler,
	.nogroup = 1,
};

static struct comm_cmd* sock_cmds[] = {
	&cmd_echo,
	&cmd_md32,
	&cmd_mw32,
	&cmd_mr32,
};

int sock_cmd_register(void)
{
	int i, r;
	struct comm_cmd *cmd;

	for(i = 0; i < ARRAY_SIZE(sock_cmds); i++) {
		cmd = sock_cmds[i];
		if (cmd) {
			comm_data_init(cmd->cdata);
			sock_data_init(cmd->priv);
			r = comm_cmd_register(cmd);
			if (r) {
				SOCK_TRACE(0, "register cmd %s failed, %d\n",
					cmd->name, r);
				break;
			}
		}
	}
	return r;
}

int sock_cmd_unregister(void)
{
	int i, r;
	struct comm_cmd *cmd;

	for(i = 0; i < ARRAY_SIZE(sock_cmds); i++) {
		cmd = sock_cmds[i];
		if (cmd) {
			r = comm_cmd_unregister(cmd);
			if (r) {
				SOCK_TRACE(0, "unregister cmd %s failed, %d\n",
					cmd->name, r);
				break;
			}
		}
	}
	return r;
}
