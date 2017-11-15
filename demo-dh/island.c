#include <stdio.h>

#define PRICE_R0	0
#define PRICE_R1	1
#define PRICE_R2	2
#define PRICE_R3	3
#define PRICE_R4	4
#define PRICE_R5	5
#define PRICE_R6	6
#define PRICE_R7	7
#define PRICE_R8	8
#define PRICE_R9	9
#define PRICE_R10	10

enum ticket {
	TK_IDLE_TO_IDLE = PRICE_R0,
	TK_IDLE_TO_TX	= PRICE_R1,
	TK_TX_TO_IDLE	= PRICE_R2,
	TK_IDLE_TO_RX	= PRICE_R3,
	TK_RX_TO_RX	= PRICE_R4,
	TK_RX_TO_IDLE	= PRICE_R5,
};

enum island {
	ISLAND_NULL,
	ISLAND_IDLE,
	ISLAND_TX,
	ISLAND_RX,
};

struct ship_ticket {
	enum island from;
	enum island to;
	enum ticket t;
};

struct ship_ticket tickets[] = {
	{ ISLAND_IDLE,	ISLAND_IDLE,	TK_IDLE_TO_IDLE	},
	{ ISLAND_IDLE,	ISLAND_TX,	TK_IDLE_TO_TX	},
	{ ISLAND_TX,	ISLAND_IDLE,	TK_TX_TO_IDLE	},
	{ ISLAND_IDLE,	ISLAND_RX,	TK_IDLE_TO_RX	},
	{ ISLAND_RX,	ISLAND_RX,	TK_RX_TO_RX	},
	{ ISLAND_RX,	ISLAND_IDLE,	TK_RX_TO_IDLE	},
};

#define ARRAY_SIZE(a) (sizeof((a)) / sizeof((a)[0]))

static enum island take_ship(enum island from, enum ticket t)
{
	int i;
	enum island to = ISLAND_NULL;
	struct ship_ticket *pt;

	for(i = 0; i < ARRAY_SIZE(tickets); i++) {
		pt = &tickets[i];
		if ((pt->from == from) && (pt->t == t)) {
			to = pt->to;
			break;
		}
	}
	return to;
}

#define ILCHAN_BUF_SIZE 0x1000

enum island_type {
	IF_NULL,
	IF_SER,
	IF_USB,
};

struct ser_conf {
	int speed;
	char data;
	char parity;
	char stop;
	char unused;
};

struct usb_conf {
	int speed;
};

struct island_chan {
	unsigned char *ptr;
	unsigned char buf[ILCHAN_BUF_SIZE];
	unsigned int cnt;
	unsigned int tm;
	unsigned int len;
	unsigned int size;
};

struct island_prop_conf {
	struct ser_conf ser;
	struct usb_conf usb;
};

struct island_prop_ctrl {
	int cmd;
	sem_t rx_ready;
	sem_t tx_ready;
	sem_t run;
	struct island_chan rx;
	struct island_chan tx;
};

struct island_prop {
	int id;
	char *name;
	enum island_type type;
	struct island_prop_ctrl ctrl;
	struct island_prop_conf conf;
};

#define prop_to_conf(p)		(&p->conf)
#define prop_to_ctrl(p)		(&p->ctrl)
#define prop_to_rxchan(p)	(&p->ctrl.rx)
#define prop_to_txchan(p)	(&p->ctrl.tx)
#define prop_to_serconf(p)	(&p->conf.ser)
#define prop_to_usbconf(p)	(&p->conf.usb)

int island_set_ser(struct island_prop *p, int speed,
			int data, int parity, int stop)
{
	struct ser_conf *ser = prop_to_serconf(p);

	ser->speed  = speed;
	ser->data   = data;
	ser->parity = parity;
	ser->stop   = stop;
	return 0;
}

int island_set_usb(struct island_prop *p, int speed)
{
	struct usb_conf *usb = prop_to_usbconf(p);

	usb->speed = speed;
	return 0;
}

enum island_cmd {
	ILCMD_INIT	= (0 << 0),
	ILCMD_TX	= (1 << 0),
	ILCMD_RX	= (1 << 1),
	ILCMD_STOP	= (1 << 2),
	ILCMD_OPEN	= (1 << 3),
	ILCMD_CLOSE	= (1 << 4),
};

int island_init_prop(struct island_prop *p,
			int id, char *name, enum IF_TYPE type)
{
	struct island_prop_ctrl *ctrl;
	struct island_prop_conf *conf;

	if (!p)
		return -1;

	ctrl = prop_to_ctrl(p);
	conf = prop_to_conf(p);

	p->id   = id;
	p->name = name;
	p->type = type;

	ctrl->cmd     = ILCMD_INIT;
	ctrl->rx.ptr  = 0;
	ctrl->rx.cnt  = 0;
	ctrl->rx.tm   = 0;
	ctrl->rx.len  = 0;
	ctrl->rx.size = IL_CHAN_BUF_SIZE;

	ctrl->tx.ptr  = 0;
	ctrl->tx.cnt  = 0;
	ctrl->tx.tm   = 0;
	ctrl->tx.len  = 0;
	ctrl->tx.size = IL_CHAN_BUF_SIZE;

	sem_init(ctrl->rx_ready, 1);
	sem_init(ctrl->tx_ready, 1);
	sem_init(ctrl->run, 1);

	if (p->type == IF_SER)
		island_set_ser(conf, 921600, 8, 'N', 1);
	else
		island_set_usb(conf, 0);

	return 0;
}

void island_stop(struct island_prop *p)
{
	struct island_prop_ctrl *ctrl = prop_to_ctrl(p);

	ctrl->cmd = ILCMD_STOP;
	sem_post(ctrl->run);
}

static int island_post_data(struct island_prop *p,
			char *txdata, int txlen, int txwait,
			char *rxdata, int rxlen, int rxwait)
{
	struct island_chan *rx, *tx;

	if (!p)
		return -1;

	if (txdata) {
		tx = prop_to_txchan(p);
		tx->ptr = txdata;
		tx->len = txlen;
		tx->cnt = 0;
		tx->tm  = txwait;
		memcpy(tx->buf, txdata, txlen);
	}
	if (rxdata) {
		rx = prop_to_rxchan(p);
		tx->ptr = rxdata;
		rx->len = rxlen;
		rx->cnt = 0;
		rx->tm  = rxwait;
	}
	return 0;
}

unsigned int island_transfer_data(struct island_prop *p, char *buf, unsigned int len,
				unsigned int wait, int cmd)
{
	unsigned int cnt;
	struct island_prop_ctrl *ctrl = prop_to_ctrl(p);

	if (cmd & ILCMD_TX) {
		island_post_data(p, buf, len, 0, 0, 0, 0);
		ctrl->cmd = ILCMD_TX;
		sem_post(ctrl->run);
		sem_wait(&p->tx_ready);
		cnt = ctrl->tx.cnt;
	}
	if (cmd & ILCMD_RX) {
		island_post_data(p, 0, 0, 0, buf, len, wait);
		ctrl->cmd = ILCMD_RX;
		sem_post(ctrl->run);
		sem_wait(&p->rx_ready);
		cnt = ctrl->rx.cnt;
	}
	return cnt;
}

void *island_work(void *data)
{
	struct island_prop *p = data; 
	struct island_prop_ctrl *ctrl = prop_to_ctrl(p);
	struct island_prop_conf *conf = prop_to_conf(p);
	enum island land = ISLAND_IDLE;
	volatile int stop = 0, cmd  = 0;

	while (1) {
		switch (land) {
		case ISLAND_IDLE:
			sem_wait(p->run);
			cmd = p->cmd;

			if (cmd & ILCMD_STOP) {
				stop = 1;
				break;
			}
			if (cmd & ILCMD_OPEN) {
			}
			if (cmd & ILCMD_CLOSE) {
			}
			if (cmd & ILCMD_TX) {
				land = take_ship(land, TK_IDLE_TO_TX);
				break;
			}
			if (cmd & ILCMD_RX) {
				land = take_ship(land, TK_IDLE_TO_RX);
				break;
			}
			land = take_ship(land, TK_IDLE_TO_IDLE);
			break;
		case ISLAND_TX:
			post_sem(p->tx_ready);
			land = take_ship(land, TK_TX_TO_IDLE);
			break;
		case ISLAND_RX:
			post_sem(p->rx_ready);
			land = take_ship(land, TK_RX_TO_IDLE);
			break;
		default:
			land = ISLAND_IDLE;
			break;
		}
		if (stop)
			break;
	}
}
