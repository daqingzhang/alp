#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int scan_dir(char *name)
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
			printf("scan end\n");
			break;
		}
		dn = entry->d_name;
		if ((!strcmp(dn, ".")) || (!strcmp(dn, ".."))) {
			continue;
		}
		lstat(dn, &ds);
		if (S_ISDIR(ds.st_mode)) {
			printf("%s\n", dn);
			scan_dir(dn);
		} else {
			printf("file: %s\n", dn);
		}
	//	printf("dir name: %s\n", entry->d_name);
	}
	chdir("..");
	closedir(dir);

	return 0;
}

int main(int argc, char *argv[])
{
	scan_dir("./abc");
	return 0;
}
