#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>

#ifdef DEBUG
#define DBG printf
#else
#define DBG(...) do{}while(0)
#endif

static const struct option long_options[] = {
	{ "help",	0, NULL, 'h' },
	{ "add",	1, NULL, 'a' },
	{ "sub",	1, NULL, 's' },
	{ "mul",	1, NULL, 'm' },
	{ "div",	1, NULL, 'd' },
	{ NULL,		0, NULL,  0  },
};

static const char *short_options = "ha:s:m:d:";
static char *prg_name;

static void print_usage(FILE *stream, int exit_code)
{
	fprintf(stream, "Usage: %s\n", prg_name);
	fprintf(stream, " -h --help		Display usage information.\n"
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

	switch(op) {
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
		case 'a':
			get_calc_param(argc, argv, &d1, &d2);
			err = do_calc(d1, d2, CALC_CMD_ADD, &res);
			if (err)
				print_usage(stderr, 1);
			stop = 1;
			break;
		case 's':
			get_calc_param(argc, argv, &d1, &d2);
			err = do_calc(d1, d2, CALC_CMD_SUB, &res);
			if (err)
				print_usage(stderr, 1);
			stop = 1;
			break;
		case 'm':
			get_calc_param(argc, argv, &d1, &d2);
			err = do_calc(d1, d2, CALC_CMD_MUL, &res);
			if (err)
				print_usage(stderr, 1);
			stop = 1;
			break;
		case 'd':
			get_calc_param(argc, argv, &d1, &d2);
			err = do_calc(d1, d2, CALC_CMD_DIV, &res);
			if (err)
				print_usage(stderr, 1);
			stop = 1;
			break;
		case '?': // invalid option
			print_usage(stderr, 1);
			break;
		case -1: // no more options are found
			break;
		default:
			abort();//abort exception, generate core dump
		}

		if (!err)
			printf("= %d\n", res);

		if (stop)
			break;
	} while(opt != -1);
	return 0;
}

int main(int argc, char *argv[])
{
	start_calc(argc, argv);

	return 0;
}
