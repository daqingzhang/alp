#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>

static int sig_cnt = 0;
static void sig_int(int signo)
{
		printf("interrupt by %d\n", signo);
		sig_cnt++;
}

void test_sig(int argc, char *argv[])
{
		printf("%s, start\n", __func__);

		signal(SIGINT, sig_int);

		while(1) {
			if (sig_cnt > 5)
				break;
		}
		printf("%s, done\n", __func__);
}

int test_dir(int argc, char *argv[])
{
		DIR *dp;
		struct dirent *dirp;

		if (argc != 2) {
				printf("invalid arguments\n");
				return -1;
		}
		char *name = argv[1];

		dp = opendir(name);
		if (!dp) {
				printf("opendir failed\n");
				return -1;
		}

		do {
				dirp = readdir(dp);
				if (dirp == NULL) {
						printf("readdir end\n");
						perror("readdir done\n");
						break;
				}
				printf("%s\n", dirp->d_name);
		} while(1);

		closedir(dp);
}

int main(int argc, char *argv[])
{
		test_sig(argc, argv);
//		test_dir(argc, argv);

		printf("done\n");
		return 0;
}

