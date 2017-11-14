#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <comm_cmd.h>

#ifdef DEBUG
#define DBG printf
#else
#define DBG(...) do{}while(0)
#endif

#define CALC_NAME "calc"
#define CALC_VER_M 1
#define CALC_VER_N 0

static const struct option long_options[] = {
	{ "help",	0, NULL, 'h' },
	{ "version",	0, NULL, 'v' },
	{ "add",	1, NULL, 'a' },
	{ "sub",	1, NULL, 's' },
	{ "mul",	1, NULL, 'm' },
	{ "div",	1, NULL, 'd' },
	{ NULL, 	0, NULL,  0  },
};

static const char *short_options = "hva:s:m:d:";
static char *prg_name;

static void calc_print_version(FILE *stream)
{
	fprintf(stream, "%s version %d.%d\n",
		CALC_NAME, CALC_VER_M, CALC_VER_N);
}

static void calc_print_usage(FILE *stream, int exit_code)
{
	fprintf(stream, "%s usage:\n", CALC_NAME);
	fprintf(stream, " -h --help		Display usage information.\n"
			" -v --version		Display version.\n"
			" -a --add [d1] [d2]	d1 + d2 \n"
			" -s --sub [d1] [d2]	d1 - d2 \n"
			" -m --mul [d1] [d2]	d1 * d2 \n"
			" -d --div [d1] [d2]	d1 / d2 \n");
//	exit(exit_code);
}

enum CALC_CMD {
	CALC_CMD_ADD,
	CALC_CMD_SUB,
	CALC_CMD_DIV,
	CALC_CMD_MUL,
};

static int calc_do_calculate(int a, int b, int op, int *res)
{
	int s = 0, r = 0;

	switch (op) {
	case CALC_CMD_ADD:
		s = a + b;
		break;
	case CALC_CMD_SUB:
		s = a - b;
		break;
	case CALC_CMD_DIV:
		if (b == 0)
			r = -1;
		s = a / b;
		break;
	case CALC_CMD_MUL:
		s = a * b;
		break;
	default:
		r = -1;
		break;
	}

	if ((!r) && (res))
		*res = s;
	return r;
}

static int calc_get_param(int argc, char *argv[], int *pa, int *pb)
{
	*pa = strtol(argv[optind - 1], NULL, 0);
	*pb = strtol(argv[optind - 0], NULL, 0);

	DBG("a=0x%x, b=0x%x\n", *pa, *pb);
	return 0;
}

static void calc_print_result(FILE *stream, int res)
{
	fprintf(stream, "%d\n", res);
}

int start_calc(int argc, char *argv[])
{
	int i;
	int err, opt;
	int d1 = 0, d2 = 0, res = 0;

	for(i = 0; i < argc; i++)
		DBG("argv[%d]: %s\n", i, argv[i]);

	prg_name = argv[0];
	optind = 1;

	do {
		opt = getopt_long(argc, argv, short_options,
						long_options, NULL);
		DBG("optind=%d\n", optind);
		switch (opt) {
		case 'h':
			calc_print_usage(stdout, 0);
			break;
		case 'v':
			calc_print_version(stdout);
			break;
		case 'a':
			calc_get_param(argc, argv, &d1, &d2);
			err = calc_do_calculate(d1, d2, CALC_CMD_ADD, &res);
			if (err)
				calc_print_usage(stderr, 1);
			calc_print_result(stdout, res);
			break;
		case 's':
			calc_get_param(argc, argv, &d1, &d2);
			err = calc_do_calculate(d1, d2, CALC_CMD_SUB, &res);
			if (err)
				calc_print_usage(stderr, 1);
			calc_print_result(stdout, res);
			break;
		case 'm':
			calc_get_param(argc, argv, &d1, &d2);
			err = calc_do_calculate(d1, d2, CALC_CMD_MUL, &res);
			if (err)
				calc_print_usage(stderr, 1);
			calc_print_result(stdout, res);
			break;
		case 'd':
			calc_get_param(argc, argv, &d1, &d2);
			err = calc_do_calculate(d1, d2, CALC_CMD_DIV, &res);
			if (err)
				calc_print_usage(stderr, 1);
			calc_print_result(stdout, res);
			break;
		case '?': // invalid option
			calc_print_usage(stderr, 1);
			break;
		case -1: // no more options are found
			break;
		default:
			abort();//abort exception, generate core dump
		}
	} while(opt != -1);
	return 0;
}

void cmd_calc_handler(int id, void *cdata, void *priv)
{
	struct comm_data *d = cdata;

	DBG("%s, cdata=%p, priv=%p\n", __func__, cdata, priv);

	start_calc(d->argc, d->argv);
}

void cmd_quit_handler(int id, void *cdata, void *priv)
{
	DBG("%s, cdata=%p, priv=%p\n", __func__, cdata, priv);
	exit(0);
}

void cmd_help_handler(int id, void *cdata, void *priv)
{
	DBG("%s, cdata=%p, priv=%p\n", __func__, cdata, priv);
	comm_show_command();
}

static struct comm_data calc_cdata;

static struct comm_cmd cmd_calc = {
	.id = 1,
	.name = "calc",
	.cdata = &calc_cdata,
	.priv = NULL,
	.handler = cmd_calc_handler,
};

static struct comm_cmd cmd_quit = {
	.id = 2,
	.name = "quit",
	.cdata = NULL,
	.priv = NULL,
	.handler = cmd_quit_handler,
};

static struct comm_cmd cmd_help = {
	.id = 2,
	.name = "help",
	.cdata = NULL,
	.priv = NULL,
	.handler = cmd_help_handler,
};

int main(int argc, char *argv[])
{
	struct comm_data *pcd;

	pcd = malloc(sizeof(struct comm_data));
	if (!pcd) {
		printf("no enough memory\n");
		return -1;
	}

	memset((void *)pcd, 0x0, sizeof(struct comm_data));

	comm_init_command();
	comm_data_init(pcd);
	comm_clear_screen();

	comm_cmd_register(&cmd_calc);
	comm_cmd_register(&cmd_quit);
	comm_cmd_register(&cmd_help);

	do {
		comm_show_screen();
		comm_get_command(pcd);
		comm_parse_command(pcd);
		comm_exec_command(pcd);
	} while (1);

	comm_cmd_unregister(&cmd_calc);
	comm_cmd_unregister(&cmd_quit);
	comm_cmd_unregister(&cmd_help);
	comm_clear_screen();

	free(pcd);
	return 0;
}
