#ifndef _XYZMODEM_H_
#define _XYZMODEM_H_

#ifdef __cplusplus
extern "C" {
#endif

struct tty {
	int  inited;
	int  (*tstc)(void);
	void (*putc)(const char c);
	void (*puts)(const char *s);
	int  (*getc)(void);
};

#define xyzModem_access   -1
#define xyzModem_noZmodem -2
#define xyzModem_timeout  -3
#define xyzModem_eof      -4
#define xyzModem_cancel   -5
#define xyzModem_frame    -6
#define xyzModem_cksum    -7
#define xyzModem_sequence -8
#define xyzModem_unknown  -9

int   getcxmodem(void);
void  xyzModem_init(struct tty *tty);
int   xyzModem_stream_open(int *err);
void  xyzModem_stream_close(int *err);
void  xyzModem_stream_terminate(int method);
int   xyzModem_stream_read(char *buf, int size, int *err);
char *xyzModem_error(int err);

#ifdef __cplusplus
}
#endif

#endif /* _XYZMODEM_H_ */
