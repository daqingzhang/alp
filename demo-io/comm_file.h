#ifndef __COMM_FILE_H__
#define __COMM_FILE_H__

struct comm_file {
	const char *name;
	const char *path;
	int fd;
	int status;
	int flag;
	int mode;
	int pos;
	int (*open)(struct comm_file *f, const char *path);
	int (*close)(struct comm_file *f);
	int (*read)(struct comm_file *f, void *buf, int nbytes, int tmsec);
	int (*write)(struct comm_file *f, const void *buf, int nbytes);
	int (*append)(struct comm_file *f, const void *buf, int nbytes);
	int (*insert)(struct comm_file *f, int pos, const void *buf, int nbytes);
	int (*at_start)(struct comm_file *f);
	int (*at_end)(struct comm_file *f);
	int (*at_mid)(struct comm_file *f, int offs);
};

struct comm_file *comm_file_create(const char *name);
void comm_file_destroy(struct comm_file *f);

#endif /* __COMMM_FILE_H__ */
