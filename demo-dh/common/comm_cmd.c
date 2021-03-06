#include <comm_cmd.h>

#define COMM_MAX_CMDS	100

static struct comm_cmd *comm_cmds[COMM_MAX_CMDS];

void comm_clear_screen(void)
{
	comm_puts("\n");
}

void comm_show_screen(void)
{
	comm_puts("CMD>");
}

void comm_show_command(void)
{
	int i;
	struct comm_cmd *cmd;

	for(i = 0; i < COMM_MAX_CMDS; i++) {
		cmd = comm_cmds[i];
		if (!cmd)
			continue;
		if ((cmd->id < 0) || (!cmd->name))
			continue;

		if (!cmd->nogroup) {
			comm_puts(cmd->group);
			comm_puts(" ");
		}
		comm_puts(cmd->name);
		comm_puts(" - ");
		if (cmd->desc)
			comm_puts(cmd->desc);
		comm_puts("\n");
	}
}

void comm_init_command(void)
{
	int i;

	for(i = 0; i < COMM_MAX_CMDS; i++)
		comm_cmds[i] = NULL;
}

int comm_data_init(struct comm_data *d)
{
	if (!d)
		return -1;
	d->argsize = COMM_ARGV_BUF_SIZE;
	d->tmpsize = COMM_TEMP_BUF_SIZE;
	d->tmpc = 0;
	d->argc = 0;
	return 0;
}

#define COMM_ASCII_SPACE 0x20
#define COMM_ASCII_TAB	 0x09
#define COMM_ASCII_EOF   '\0'

int comm_get_command(struct comm_data *d)
{
	char *temp = d->temp;
	int size = d->tmpsize;
	int c, cnt = 0;

	while (1) {
		/* comm_fgetc() should be blocked until data come */
		c = comm_fgetc();
		if ((c == '\n')) {
			if (cnt > 0) {
				if (temp[cnt - 1] == '\r')
					--cnt;
			}
			temp[cnt] = COMM_ASCII_EOF;
			break;
		}
		if (cnt >= size)
			cnt = 0;
		temp[cnt++] = c;
	}
	d->tmpc = cnt;

	COMM_TRACE(1, "%s, temp[%p]:%s\n", __func__, temp, temp);
	return 0;
}

int comm_parse_command(struct comm_data *d)
{
	char *temp = d->temp;
	int tmpc = d->tmpc;
	int argi = 0;
	int tmpi = 0;
	int i, r = 0;

	while (1) {
		if ((tmpi >= tmpc) || (temp[tmpi] == COMM_ASCII_EOF)) {
			d->argc = argi;
			break;
		}
		if ((temp[tmpi] == COMM_ASCII_SPACE)
			|| (temp[tmpi] == COMM_ASCII_TAB)) {
			tmpi++;
			continue;
		}
		d->argv[argi++] = &temp[tmpi];
		while (1) {
			if (tmpi >= tmpc)
				break;
			if ((temp[tmpi] == COMM_ASCII_SPACE)
				|| (temp[tmpi] == COMM_ASCII_TAB)) {
				temp[tmpi] = COMM_ASCII_EOF;
				tmpi++;
				break;
			}
			tmpi++;
		}
	}

	for(i = 0; i < argi; i++)
		COMM_TRACE(1, "%s, argc=%d, argv[%p]:%s\n", __func__,
			d->argc, d->argv[i], d->argv[i]);
	return r;
}

int comm_exec_command(struct comm_data *d)
{
	int i;
	char *cmd_name;
	char *cmd_group;

	if (d->argc <= 0) {
		COMM_TRACE(1, "%s, argc=%d\n", __func__, d->argc);
		return -1;
	}

	COMM_TRACE(1, "%s, argv[0]:%s, argv[1]:%s\n", __func__,
		d->argv[0], d->argv[1]);

	for(i = 0;i < COMM_MAX_CMDS;i++) {
		struct comm_cmd *cmd;

		cmd = comm_cmds[i];

		if(!cmd)
			continue;

		if ((cmd->id < 0) || (!cmd->name))
			continue;

		if (cmd->nogroup == 1) {
			cmd_name = d->argv[0];
		} else {
			cmd_group= d->argv[0];
			cmd_name = d->argv[1];
		}

		if (cmd->nogroup == 0) {
			if (strcmp(cmd_group, cmd->group)) {
				continue;
			}
		}
		if (!strcmp(cmd_name, cmd->name)) {
			COMM_TRACE(1, "%s, cmd: %s, %d, %p, %p, %p\n", __func__,
				cmd->name, cmd->id, cmd->handler,
				cmd->cdata, cmd->priv);

			if (cmd->cdata)
				memcpy(cmd->cdata, d, sizeof(struct comm_data));
			if (cmd->handler)
				cmd->handler(cmd->id, (void *)(cmd->cdata), cmd->priv);

			break;
		}
	}
	return 0;
}

#define COMM_ERR_CMD	2
#define COMM_ERR_ID 	3
#define COMM_ERR_NAME	4
#define COMM_ERR_POS	5

static int comm_check_command(struct comm_cmd *cmd)
{
	if (!cmd) {
		COMM_TRACE(0, "null cmd\n");
		return -COMM_ERR_CMD;
	}
	if (cmd->id < 0) {
		COMM_TRACE(0, "cmd id %d, should be 0 ~ %d\n",
			cmd->id, COMM_MAX_CMDS);
		return -COMM_ERR_ID;
	}
	if (!cmd->name) {
		COMM_TRACE(0, "cmd name is null\n");
		return -COMM_ERR_NAME;
	}
	return 0;
}

static int comm_match_command(struct comm_cmd *c)
{
	int i;

	for(i = 0; i < COMM_MAX_CMDS; i++) {
		struct comm_cmd *cmd;

		cmd = comm_cmds[i];

		if (cmd == c) {
			if (!c)
				return i;
			if (!strcmp(cmd->name, c->name))
				return i;
		}
	}
	return -1;
}

#define comm_find_empty_command() comm_match_command(0)

static int comm_set_command(struct comm_cmd *cmd, int pos)
{
	if (pos >= COMM_MAX_CMDS)
		return -COMM_ERR_POS;
	comm_cmds[pos] = cmd;
	return 0;
}

int comm_cmd_register(struct comm_cmd *cmd)
{
	int pos, r;

	r = comm_check_command(cmd);
	if (r)
		return r;

	pos = comm_find_empty_command();
	if (pos < 0)
		return -1;

	r = comm_set_command(cmd, pos);
	if (r)
		return r;

	if (cmd->nogroup) {
		COMM_TRACE(1, "register cmd: %s, %4d, handler[%p]\n",
			cmd->name, cmd->id, cmd->handler);
	} else {
		COMM_TRACE(1, "register cmd: %s %s, %4d, handler[%p]\n",
			cmd->group, cmd->name, cmd->id, cmd->handler);
	}
	return 0;
}

int comm_cmd_unregister(struct comm_cmd *cmd)
{
	int pos, r;

	r = comm_check_command(cmd);
	if (r)
		return r;

	pos = comm_match_command(cmd);
	if (pos < 0)
		return -1;

	r = comm_set_command(NULL, pos);
	if (r)
		return r;

	COMM_TRACE(1, "unregister cmd: %s, %d, handler[%p]\n",
		cmd->name, cmd->id, cmd->handler);
	return 0;
}
