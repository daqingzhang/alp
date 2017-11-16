#ifndef __SERIAL_THREAD_H__
#define __SERIAL_THREAD_H__
#include <pthread.h>
#include <semaphore.h>

#define SER_CONF_DEFAULT_BAUD	921600
#define SER_CONF_DEFAULT_DATA	8
#define SER_CONF_DEFAULT_PARITY	'N'
#define SER_CONF_DEFAULT_STOP	1

#define SERIAL_CHAN_BUF_SIZE 0x2000

struct serial_conf {
	const char *name;
	int  speed;
	char data;
	char parity;
	char stop;
	char undef;
};


struct serial_chan {
	unsigned char buf[SERIAL_CHAN_BUF_SIZE];
	unsigned int cnt;
	unsigned int len;
	unsigned int size;
	unsigned int timeout;
};

struct serial_ctrl {
	int cmd;
	int open;
	int stop;
	sem_t rx_ready;
	sem_t tx_ready;
	sem_t active;
	pthread_t pid;
	pthread_attr_t attr;
	pthread_mutext_t mutex;
	void (*run)(void *data);
};

struct serial_prop {
	int id;
	const char *name;
	struct serial_conf conf;
	struct serial_ctrl ctrl;
	struct serial_chan rxchan;
	struct serial_chan txchan;
};

struct serial_obj {
	struct serial_prop prop;
	int (*init)(struct serial_obj *obj, const char *name);
	int (*open)(struct serial_obj *obj, const char *port);
	void (*close)(struct serial_obj *obj);
	void (*debug)(struct serial_obj *obj);
	unsigned int (*read)(struct serial_obj *obj, char *pbuf,
			unsigned int len, unsigned int timeout);
	unsigned int (*write)(struct serial_obj *obj, const char *pbuf,
			unsigned int len);
};

struct serial_obj *serial_create(const char *name);
void serial_destroy(struct serial_obj *obj);

#endif /* __SERIAL_THREAD_H__ */
