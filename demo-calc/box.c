#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <comm_cmd.h>
#include <calc.h>

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

static struct comm_cmd cmd_quit = {
	.id = 99,
	.name = "quit",
	.desc = "exit the program",
	.cdata = NULL,
	.priv = NULL,
	.handler = cmd_quit_handler,
};

static struct comm_cmd cmd_help = {
	.id = 100,
	.name = "help",
	.desc = "show help info",
	.cdata = NULL,
	.priv = NULL,
	.handler = cmd_help_handler,
};

int box_cmd_register(void)
{
	int r;

	r = comm_cmd_register(&cmd_quit);
	r += comm_cmd_register(&cmd_help);

	return r;
}

int box_cmd_unregister(void)
{
	int r;

	r = comm_cmd_unregister(&cmd_quit);
	r += comm_cmd_unregister(&cmd_help);

	return r;
}

int main(int argc, char *argv[])
{
	struct comm_data *pcd;

	box_quit = 0;

	pcd = malloc(sizeof(struct comm_data));
	if (!pcd) {
		printf("no enough memory\n");
		return -1;
	}

	memset((void *)pcd, 0x0, sizeof(struct comm_data));

	comm_init_command();
	comm_data_init(pcd);
	comm_clear_screen();

	box_cmd_register();
	calc_cmd_register();

	do {
		comm_show_screen();
		comm_get_command(pcd);
		comm_parse_command(pcd);
		comm_exec_command(pcd);
	} while (!box_quit);

	box_cmd_unregister();
	calc_cmd_unregister();

	comm_clear_screen();

	free(pcd);

	printf("See you again.\n");
	return 0;
}
