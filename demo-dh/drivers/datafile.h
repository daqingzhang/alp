#ifndef __DATAFILE_H__
#define __DATAFILE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

enum OFLAG {
	OFLAG_RO, //read only
	OFLAG_WO, //write only
	OFLAG_WR, //write and read
	OFLAG_AP, //append
};

class clsDataFile {
private:
	const char *f_name;
	int f_opened;
	enum OFLAG f_flag;
	FILE *f_stream;
public:
	clsDataFile();
	~clsDataFile();
public:
	int fileOpen(const char *pathname, enum OFLAG flag);
	void fileClose(void);
	uint32_t fileRead(char *buf, uint32_t bytes);
	uint32_t fileWrite(const char *data, uint32_t bytes);
	int fileFlush(void);
	int atStart(void);
	int atEnd(void);
	int atPos(uint32_t new_pos);
	uint32_t getPos(void);
	uint32_t getSize(void);
	int isError(void);
	int isEnd(void);
};

#ifdef __cplusplus
}
#endif

#endif
