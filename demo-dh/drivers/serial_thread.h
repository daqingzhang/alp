#ifndef __SERIAL_THREAD_H__
#define __SERIAL_THREAD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <oslib.h>

#define SER_CONF_DEFAULT_BAUD	921600
#define SER_CONF_DEFAULT_DATA	8
#define SER_CONF_DEFAULT_PARITY	'N'
#define SER_CONF_DEFAULT_STOP	1

#define SERIAL_CHAN_BUF_SIZE 0x2000

struct serial_conf {
	const char *port;
	int  speed;
	char data;
	char parity;
	char stop;
	char undef;
};

struct serial_chan {
	char buf[SERIAL_CHAN_BUF_SIZE];
	unsigned int cnt;
	unsigned int len;
	unsigned int size;
	unsigned int timeout;
};

struct serial_ctrl {
	int cmd;
	int stop;
	int status;
	int fd;
	sem_t rx_ready;
	sem_t tx_ready;
	sem_t active;
	pthread_t pid;
	pthread_attr_t attr;
	pthread_mutex_t mutex;
	void* (*run)(void *data);
};

struct serial_prop {
	int id;
	const char *name;
	struct serial_conf conf;
	struct serial_ctrl ctrl;
	struct serial_chan rxchan;
	struct serial_chan txchan;
};

struct serial_dev {
	struct serial_prop prop;
	int (*open)(struct serial_dev *dev, const char *port_name);
	void (*close)(struct serial_dev *dev);
	int (*set_speed)(struct serial_dev *dev, int baud);
	int (*config)(struct serial_dev *dev,
			int baud, int data, char parity, int stop);
	int (*read)(struct serial_dev *dev,
			char *pbuf, unsigned int len, unsigned int tm_sec);
	int (*write)(struct serial_dev *dev,
			const char *pbuf, unsigned int len);
};

struct serial_dev *serial_dev_create(const char *name);
void serial_dev_destroy(struct serial_dev *dev);

#ifdef __cplusplus
}
#endif

#endif /* __SERIAL_THREAD_H__ */
