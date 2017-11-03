#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>

static const struct option long_options[] = {
	{ "help",	0, NULL, 'h' },
	{ "output",	1, NULL, 'o' },
	{ "verbose",	0, NULL, 'v' },
	{ NULL,		0, NULL,  0  },
};

static const char *short_options = "ho:v";
static char *prg_name;

static void print_usage(FILE *stream, int exit_code)
{
	fprintf(stream, "Usage: %s\n", prg_name);
	fprintf(stream, " -h --help		Display usage information.\n"
			" -o --output filename	Write output to file.\n"
			" -v --verbose		Print verbose messages.\n");
	exit(exit_code);
}

int test_opt(int argc, char *argv[])
{
	int next_option;
	char *outfile;
	int verbose = 0;

	prg_name = argv[0];

	do {
		next_option = getopt_long(argc, argv, short_options,
						long_options, NULL);
		switch (next_option) {
		case 'h':
			print_usage(stdout, 0);
			break;
		case 'v':
			verbose = 1;
			break;
		case 'o':
			outfile = optarg;
			fprintf(stdout, "outfile: %s\n", outfile);
			break;
		case '?': // invalid option
			print_usage(stderr, 1);
			break;
		case -1: // no more options are found
			break;
		default:
			abort();//abort exception, generate core dump
		}
	} while(next_option != -1);

	if (verbose) {
		int i;
		for(i = optind; i < argc; i++)
			fprintf(stdout, "argv[%d]: %s\n", i, argv[i]);
	}
	return 0;
}

int loop_arg(int argc, char *argv[])
{
	int i;
	char *p;

	printf("argc=%d\n", argc);
	if (argc > 0) {
		char *fname = argv[0];

		printf("file name: %s\n", fname);
		for(i = 1;i < argc; i++) {
			p = argv[i];
			printf("argv[%d]: %s\n", i, p);
		}
	}
	return 0;
}


int test_ioe(int argc, char *argv[])
{
	printf("start\n");
#if 0
	while (1) {
		printf(".\n");
		sleep(1);
	}
#endif

#if 1
	while (1) {
		fprintf(stdout, ".");
		sleep(1);
	}
#endif

#if 0
	while (1) {
		/*
		 * stderr is not buffered, it will send data
		 * to console directly
		 */
		fprintf(stderr, ".");
		sleep(1);
	}
#endif

#if 0
	while (1) {
		/*
		 * stdout is buffered, it will send data to console
		 * until buffered is filled('\n' is checked)
		 */
		printf(".");
		fflush(stdout);
		sleep(1);
	}
#endif

	printf("end\n");
	return 0;
}

int test_env(int argc, char *argv[])
{
	const char *penv, *pval;

	if (argc > 0)
		printf("prg name: %s\n", argv[0]);

	if (argc > 1) {
		penv = argv[1];
		pval = getenv(penv);
		printf("old env %s = %s\n", penv, pval);
	} else {
		penv = "ABCDEF";
	}

	if (argc > 2)
		pval = argv[2];
	else
		pval = "goodcode";

	printf("set env %s = %s\n", penv, pval);
	setenv(penv, pval, 1);

	pval = getenv(penv);
	printf("get env %s = %s\n", penv, pval);
	return 0;
}


int test_opt(int argc, char *argv[]);
int loop_arg(int argc, char *argv[]);
int test_ioe(int argc, char *argv[]);
int test_env(int argc, char *argv[]);

int main(int argc, char *argv[])
{
//	test_opt(argc, argv);
//	loop_arg(argc, argv);
//	test_ioe(argc, argv);
	test_env(argc, argv);

	return 0;
}
