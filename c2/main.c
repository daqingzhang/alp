#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

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

int main(int argc, char *argv[])
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
		case '?':
			print_usage(stderr, 1);
			break;
		case -1:
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

#if 0
int main(int argc, const char *argv[])
{
	int i;
	const char *p;

	printf("argc=%d\n", argc);
	if (argc > 0) {
		const char *fname = argv[0];

		printf("file name: %s\n", fname);
		for(i = 1;i < argc; i++) {
			p = argv[i];
			printf("argv[%d]: %s\n", i, p);
		}
	}
	return 0;
}
#endif
