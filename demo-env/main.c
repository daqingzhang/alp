#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

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

int search_dir(const char *dir_name, const char *file_name, char *abs_path)
{
	int r = 0;
	char *dname;
	DIR *dir;
	struct dirent *entry;
	struct stat dstat;

	dir = opendir(dir_name);
	if (!dir) {
		perror("opendir error:");
		return -1;
	}
	chdir(dir_name);
	while (1) {
		entry = readdir(dir);
		if (!entry) {
			break;
		}
		dname = entry->d_name;
		if ((!strcmp(dname, ".")) || (!strcmp(dname, ".."))) {
			continue;
		}
		if (!strcmp(dname, file_name)) {
			realpath(dname, abs_path);
			printf("find %s: %s\n", file_name, abs_path);
			r = 1;
			break;
		}
		lstat(dname, &dstat);
		if (S_ISDIR(dstat.st_mode)) {
			r = search_dir(dname, file_name, abs_path);
			if (r)
				break;
		}
	}
	chdir("..");
	return r;
}

int get_path(void)
{
	char abs_path[256];

	memset(abs_path, 0x0, sizeof(abs_path));
	realpath(".", abs_path);
	printf("abs_path: %s\n", abs_path);

	chdir("..");
	memset(abs_path, 0x0, sizeof(abs_path));
	realpath(".", abs_path);
	printf("abs_path: %s\n", abs_path);
	return 0;
}

int main(int argc, char *argv[])
{
	int r;
	char *dir_name, *file_name;
	char abs_path[256];

	memset(abs_path, 0x0, sizeof(abs_path));

//	get_path();

	if (argc > 1)
		dir_name = argv[1];
	else
		dir_name = "./abc";
	if (argc > 2)
		file_name = argv[2];
	else
		file_name = "c.c";

	if (argc == 3) {
		r = search_dir(dir_name, file_name, abs_path);
		if (!r)
			printf("not find %s\n", file_name);
		else
			printf("file path: %s\n", abs_path);
	} else {
		r = scan_dir(dir_name, 0);
		if (r)
			printf("scan dir %s failed\n", dir_name);
		else
			printf("scan dir %s okay\n", dir_name);
	}
	return r;
}
