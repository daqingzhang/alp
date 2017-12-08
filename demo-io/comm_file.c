#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include "comm_file.h"

#ifdef DEBUG
#define DBG printf
#else
#define DBG(...) do{}while(0)
#endif

static int comm_file_open(struct comm_file *f, const char *path)
{
	int fd;
	int flag = 0, find = 0;
	mode_t m = 0;

	if (f->status)
		return 0;

	if (!path)
		return -1;

	fd = open(path, O_RDONLY);
	if (fd > 0) {
		find = 1;
		close(fd);
	}

	flag = O_RDWR;
	if (!find)
		flag |= O_CREAT;

	if (find) {
		fd = open(path, flag);
	} else {
		m = 0664;
		fd = open(path, flag, m);
	}
	if (fd < 0) {
		perror("open file failed\n");
	}
	f->fd = fd;
	f->mode = m;
	f->path = path;
	f->flag = flag;
	f->status = 1;

	DBG("%s fd=%d\n", __func__, fd);
	return 0;
}

static int comm_file_close(struct comm_file *f)
{
	int r;

	if (f->status) {
		f->status = 0;
		r = close(f->fd);
	}
	DBG("%s r=%d\n", __func__, r);
	return r;
}

#if 0
static int file_getc(int fd, char *c)
{
	return read(fd, c, 1);
}
#endif

#define DLY_SLOT 20
#define DLY_CNT(sec) (sec * 1000 / DLY_SLOT)

static inline void fdelay(void)
{
	usleep(DLY_SLOT);
}

static int comm_file_read(struct comm_file *f, void *buf, int nbytes, int tmsec)
{
	int fd = f->fd;
	int len, last, n;
	int us_cnt = DLY_CNT(tmsec);
	char *pd = buf;

	if (!f->status)
		return 0;

	last = nbytes;
	len = 0;

	while(1) {
		if (!last)
			break;

		n = read(fd, &pd[len], last);
		if (n == -1) {
			perror("read error\n");
		} else if (n > 0) {
			len += n;
			last -= n;
		} else {
			if (!us_cnt)
				break;
			us_cnt--;
			fdelay();
		}
	}
	DBG("%s len=%d\n", __func__ ,len);
	return len;
}

static int comm_file_write(struct comm_file *f, const void *buf, int nbytes)
{
	if (!f->status)
		return 0;

	return write(f->fd, buf, nbytes);
}

static int comm_file_at_start(struct comm_file *f)
{
	if (!f->status)
		return -1;

	f->pos = lseek(f->fd, 0, SEEK_SET);

	DBG("%s, pos=%d\n", __func__, f->pos);
	return f->pos;
}

static int comm_file_at_end(struct comm_file *f)
{
	if (!f->status)
		return -1;

	f->pos = lseek(f->fd, 0, SEEK_END);

	DBG("%s, pos=%d\n", __func__, f->pos);
	return f->pos;
}

static int comm_file_at_mid(struct comm_file *f, int offs)
{
	if (!f->status)
		return -1;

	f->pos = lseek(f->fd, offs, SEEK_SET);

	DBG("%s, pos=%d\n", __func__, f->pos);
	return f->pos;
}

static int comm_file_append(struct comm_file *f, const void *buf, int nbytes)
{
	int r;

	DBG("%s, nbytes=%d\n", __func__, nbytes);
	r = comm_file_at_end(f);
	if (r < 0)
		return r;

	return comm_file_write(f, buf, nbytes);
}

static int comm_file_insert(struct comm_file *f, int pos, const void *buf, int nbytes)
{
	int r;

	DBG("%s, pos=%d, nbytes=%d\n", __func__, pos, nbytes);
	r = comm_file_at_mid(f, pos);
	if (r < 0)
		return r;

	return comm_file_write(f, buf, nbytes);
}

static int comm_file_init(struct comm_file *f)
{
	memset(f, 0x0, sizeof(struct comm_file));
	return 0;
}

struct comm_file *comm_file_create(const char *name)
{
	struct comm_file *f;

	f = malloc(sizeof(struct comm_file));
	if (!f) {
		return 0;
	}

	comm_file_init(f);

	f->name = name;
	f->open = comm_file_open;
	f->close = comm_file_close;
	f->read = comm_file_read;
	f->write = comm_file_write;
	f->at_start = comm_file_at_start;
	f->at_end = comm_file_at_end;
	f->at_mid = comm_file_at_mid;
	f->append = comm_file_append;
	f->insert = comm_file_insert;

	DBG("%s %s (%p) okay\n", __func__, name, f);
	return f;
}

void comm_file_destroy(struct comm_file *f)
{
	if (!f)
		return;

	if (f->status)
		f->close(f);

	DBG("%s %s (%p) okay\n", __func__, f->name, f);
	free(f);
}
