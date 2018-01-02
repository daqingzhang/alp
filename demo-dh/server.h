#ifndef __SERVER_H__
#define __SERVER_H__

int sock_getc(void);
int sock_getc_blocked(void);
int sock_putc(char c);
int sock_read(char *buf, int len);
int sock_read_blocked(char *buf, int len);
int sock_write(const char *buf, int len);

#endif
