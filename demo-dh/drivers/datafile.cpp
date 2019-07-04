#include <stdio.h>
#include <datafile.h>

#ifdef DEBUG
#define DFILE_TRACE printf
#else
#define DFILE_TRACE(...) do{}while(0)
#endif

//#define DFILE_TRACE(...) do{ }while(0)
#define FILE_ERR printf

clsDataFile::clsDataFile()
{
	f_name  = NULL;
	f_opened= 0;
	f_flag  = OFLAG_RO;
}

clsDataFile::~clsDataFile()
{
	fileClose();
}

int clsDataFile::fileOpen(const char *pathname, enum OFLAG flag)
{
	const char *m;

	if (!pathname) {
		DFILE_TRACE("null pathname\n");
		return -1;
	}
	if (f_opened) {
		DFILE_TRACE("file already opened\n");
		return -2;
	}
	switch(flag) {
	case OFLAG_RO:
		m = "r";
		break;
	case OFLAG_WO:
		m = "a";//write only,
		break;
	case OFLAG_WR:
		m = "r+";
		break;
	case OFLAG_AP:
	default:
		m = "a+";
		break;
	}

	f_stream = fopen(pathname, m);
	if (!f_stream) {
		DFILE_TRACE("open file %s failed\n", pathname);
		return -3;
	}
	f_opened = 1;
	f_name = pathname;
	f_flag = flag;

	DFILE_TRACE("%s, name:%s, flag:%d\n",
		__func__, f_name, f_flag);
	return 0;
}

void clsDataFile::fileClose(void)
{
	if (f_opened) {
		int r;

		r = fclose(f_stream);
		f_opened = 0;
		if (r) {
			DFILE_TRACE("close file error %d\n", r);
			return;
		}
	}
}

uint32_t clsDataFile::fileRead(char *buf, uint32_t bytes)
{
	uint32_t len = 0;

	if (!bytes) {
		DFILE_TRACE("%s, zero bytes\n", __func__);
		return 0;
	}
	if (!buf) {
		DFILE_TRACE("%s, null buffer\n", __func__);
		return 0;
	}
	if (!f_opened) {
		DFILE_TRACE("%s, not opened\n", __func__);
		return 0;
	}
	if (f_flag == OFLAG_WO) {
		DFILE_TRACE("%s, oflag=%d error\n", __func__, f_flag);
		return 0;
	}

	len = fread((void *)buf, 1, bytes, f_stream);
	if (len == 0) {
		DFILE_TRACE("%s, read error\n", __func__);
		return 0;
	}
	if (len != bytes) {
		DFILE_TRACE("%s, lost %d B data\n", __func__, bytes - len);
	}
	return len;
}

uint32_t clsDataFile::fileWrite(const char *buf, uint32_t bytes)
{
	uint32_t len = 0;

	if (!bytes) {
		DFILE_TRACE("%s, zero bytes\n", __func__);
		return 0;
	}
	if (!buf) {
		DFILE_TRACE("%s, null buffer\n", __func__);
		return 0;
	}
	if (!f_opened) {
		DFILE_TRACE("%s, not opened\n", __func__);
		return 0;
	}
	if (f_flag == OFLAG_RO) {
		DFILE_TRACE("%s, oflag=%d error\n", __func__, f_flag);
		return 0;
	}
	len = fwrite((const void *)buf, 1, bytes, f_stream);
	if (len == 0) {
		DFILE_TRACE("%s, read error\n", __func__);
		return 0;
	}
	if (len != bytes) {
		DFILE_TRACE("%s, lost %d B data\n", __func__, bytes - len);
	}
	return len;
}

int clsDataFile::fileFlush(void)
{
	int r = 0;

	if (f_opened)
		r = fflush(f_stream);

	return r;
}

int clsDataFile::atStart(void)
{
	int r;

	r = fseek(f_stream, 0, SEEK_SET);
	if (r) {
		DFILE_TRACE("%s, error %d\n", __func__, r);
	}
	return r;
}

int clsDataFile::atEnd(void)
{
	int r;

	r = fseek(f_stream, 0, SEEK_END);
	if (r) {
		DFILE_TRACE("%s, error %d\n", __func__, r);
	}
	return r;
}

int clsDataFile::atPos(uint32_t new_pos)
{
	int r;

	r = fseek(f_stream, new_pos, SEEK_SET);
	if (r) {
		DFILE_TRACE("%s, error %d\n", __func__, r);
	}
	return r;
}

uint32_t clsDataFile::getPos(void)
{
	uint32_t r;

	r = ftell(f_stream);
	return r;
}

uint32_t clsDataFile::getSize(void)
{
	int r;
	uint32_t len, cur_len;

	cur_len = ftell(f_stream);

	r = fseek(f_stream, 0, SEEK_END);
	if (r) {
		DFILE_TRACE("%s, error %d\n", __func__, r);
	}
	len = ftell(f_stream);

	r = fseek(f_stream, cur_len, SEEK_SET);
	if (r) {
		DFILE_TRACE("%s, error %d\n", __func__, r);
	}
	return len;
}

int clsDataFile::isError(void)
{
	int r;

	r = ferror(f_stream);
	if (r) {
		clearerr(f_stream);
	}

	DFILE_TRACE("%s, r=%d\n", __func__, r);
	return r;
}

int clsDataFile::isEnd(void)
{
	int r;

	r = feof(f_stream);
	DFILE_TRACE("%s, r=%d\n", __func__, r);
	return r;
}

