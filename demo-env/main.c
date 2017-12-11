#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void print_indent(int depth, char prefix, char *name, char postfix)
{
	int i;

	for(i = 0; i < depth; i++)
		printf("%c", prefix);

	if (postfix)
		printf("%s%c\n", name, postfix);
	else
		printf("%s\n", name);
}

int scan_dir(char *name, int depth)
{
	char *dn;
	DIR *dir;
	struct dirent *entry;
	struct stat ds;

	dir = opendir(name);
	if (!dir) {
		perror("opendir error:");
		return -1;
	}
	chdir(name);
	while(1) {
		entry = readdir(dir);
		if (!entry) {
		//	printf("scan end, %d\n", depth);
			break;
		}
		dn = entry->d_name;
		if ((!strcmp(dn, ".")) || (!strcmp(dn, ".."))) {
			continue;
		}
		lstat(dn, &ds);
		if (S_ISDIR(ds.st_mode)) {
			print_indent(depth + 4, ' ', dn, '/');
			scan_dir(dn, depth + 4);
		} else {
			print_indent(depth + 4, ' ', dn, 0);
		}
	//	printf("dir name: %s\n", entry->d_name);
	}
	chdir("..");
	closedir(dir);

	return 0;
}

int main(int argc, char *argv[])
{
	char *path;

	if (argc > 1)
		path = argv[1];
	else
		path = "./abc";

	scan_dir(path, 0);
	return 0;
}
