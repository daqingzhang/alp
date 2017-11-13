#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>

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

static void print_version(FILE *stream)
{
	fprintf(stream, "%s version %d.%d\n",
		CALC_NAME, CALC_VER_M, CALC_VER_N);
}

static void print_usage(FILE *stream, int exit_code)
{
	fprintf(stream, "%s usage:\n", CALC_NAME);
	fprintf(stream, " -h --help		Display usage information.\n"
			" -v --version		Display version.\n"
			" -a --add [d1] [d2]	d1 + d2 \n"
			" -s --sub [d1] [d2]	d1 - d2 \n"
			" -m --mul [d1] [d2]	d1 * d2 \n"
			" -d --div [d1] [d2]	d1 / d2 \n");
	exit(exit_code);
}

enum CALC_CMD {
	CALC_CMD_ADD,
	CALC_CMD_SUB,
	CALC_CMD_DIV,
	CALC_CMD_MUL,
};

static int do_calc(int a, int b, int op, int *res)
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

static int get_calc_param(int argc, char *argv[], int *pa, int *pb)
{
	*pa = strtol(argv[optind - 1], NULL, 0);
	*pb = strtol(argv[optind - 0], NULL, 0);

	DBG("a=0x%x, b=0x%x\n", *pa, *pb);
	return 0;
}

static void print_calc_res(FILE *stream, int res)
{
	fprintf(stream, "%d\n", res);
}

int start_calc(int argc, char *argv[])
{
	int i, err, stop = 0, opt;
	int d1 = 0, d2 = 0, res = 0;

	for(i = 0; i < argc; i++)
		DBG("argv[%d]: %s\n", i, argv[i]);

	prg_name = argv[0];

	do {
		opt = getopt_long(argc, argv, short_options,
						long_options, NULL);
		DBG("optind=%d\n", optind);
		switch (opt) {
		case 'h':
			print_usage(stdout, 0);
			break;
		case 'v':
			print_version(stdout);
			break;
		case 'a':
			get_calc_param(argc, argv, &d1, &d2);
			err = do_calc(d1, d2, CALC_CMD_ADD, &res);
			if (err)
				print_usage(stderr, 1);
			print_calc_res(stdout, res);
			break;
		case 's':
			get_calc_param(argc, argv, &d1, &d2);
			err = do_calc(d1, d2, CALC_CMD_SUB, &res);
			if (err)
				print_usage(stderr, 1);
			print_calc_res(stdout, res);
			break;
		case 'm':
			get_calc_param(argc, argv, &d1, &d2);
			err = do_calc(d1, d2, CALC_CMD_MUL, &res);
			if (err)
				print_usage(stderr, 1);
			print_calc_res(stdout, res);
			break;
		case 'd':
			get_calc_param(argc, argv, &d1, &d2);
			err = do_calc(d1, d2, CALC_CMD_DIV, &res);
			if (err)
				print_usage(stderr, 1);
			print_calc_res(stdout, res);
			break;
		case '?': // invalid option
			print_usage(stderr, 1);
			break;
		case -1: // no more options are found
			break;
		default:
			abort();//abort exception, generate core dump
		}

		if (stop)
			break;
	} while(opt != -1);
	return 0;
}

int parse_arguments(const char *s, char **argv[])
{
	return 0;
}

#define CALC_TMP_BUF_SIZE 2000
#define CALC_CMD_BUF_SIZE 100

struct calc_data {
	char	tmp[CALC_TMP_BUF_SIZE];
	char*	cmd[CALC_CMD_BUF_SIZE];
	int	cmdlen;
	int	tmplen;
	int 	tmpsize;
	int 	cmdsize;
};

void init_calc_data(struct calc_data *pd)
{
	pd->cmdsize = CALC_CMD_BUF_SIZE;
	pd->tmpsize = CALC_TMP_BUF_SIZE;
	pd->tmplen = 0;
	pd->cmdlen = 0;
}

void print_screen(void)
{

}

int input_cmd(struct calc_data *pd)
{
	return 0;
}

int parse_cmd(struct calc_data *pd)
{
	return 0;
}

int main(int argc, char *argv[])
{
	int quit = 0;
	struct calc_data *pcd;

	pcd = malloc(sizeof(struct calc_data));

	init_calc_data(pcd);

	do {
		print_screen();
		input_cmd(pcd);
		quit = parse_cmd(pcd);
		if (!quit)
			start_calc(pcd->cmdlen, pcd->cmd);
	} while (!quit);

	free(pcd);

	return 0;
}
