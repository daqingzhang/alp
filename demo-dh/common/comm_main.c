#include <comm_cmd.h>

static int box_quit = 0;

static void cmd_quit_handler(int id, void *cdata, void *priv)
{
	DBG("%s, cdata=%p, priv=%p\n", __func__, cdata, priv);
//	exit(0);
	box_quit = 1;
}

static void cmd_help_handler(int id, void *cdata, void *priv)
{
	DBG("%s, cdata=%p, priv=%p\n", __func__, cdata, priv);
	comm_show_command();
}

static struct comm_cmd cmd_exit = {
	.id = 98,
	.name = "exit",
	.desc = "exit the program",
	.cdata = NULL,
	.priv = NULL,
	.handler = cmd_quit_handler,
	.nogroup = 1,
};

static struct comm_cmd cmd_quit = {
	.id = 99,
	.name = "quit",
	.desc = "quit the program",
	.cdata = NULL,
	.priv = NULL,
	.handler = cmd_quit_handler,
	.nogroup = 1,
};

static struct comm_cmd cmd_help = {
	.id = 100,
	.name = "help",
	.desc = "show help info",
	.cdata = NULL,
	.priv = NULL,
	.handler = cmd_help_handler,
	.nogroup = 1,
};

int trivial_cmd_register(void)
{
	int r;

	r = comm_cmd_register(&cmd_quit);
	r += comm_cmd_register(&cmd_exit);
	r += comm_cmd_register(&cmd_help);

	return r;
}

int trivial_cmd_unregister(void)
{
	int r;

	r = comm_cmd_unregister(&cmd_quit);
	r += comm_cmd_register(&cmd_exit);
	r += comm_cmd_unregister(&cmd_help);

	return r;
}

int comm_main(int argc, char *argv[])
{
	int r;
	struct comm_data box_cdata;
	struct comm_data *pcd = &box_cdata;

	box_quit = 0;

	memset((void *)pcd, 0x0, sizeof(struct comm_data));

	comm_init_command();
	comm_data_init(pcd);
	comm_clear_screen();

	r = trivial_cmd_register();
	r += comm_user_cmd_register();
	if (r) {
		ioprintf("register cmd failed %d\n", r);
		return r;
	}

	do {
		comm_show_screen();
		comm_get_command(pcd);
		comm_parse_command(pcd);
		comm_exec_command(pcd);
	} while (!box_quit);

	r = comm_user_cmd_unregister();
	r += trivial_cmd_unregister();
	if (r) {
		ioprintf("unregister cmd failed %d\n", r);
		return r;
	}

	comm_clear_screen();

	ioprintf("See you again.\n");
	return 0;
}
