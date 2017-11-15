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

enum island take_ship(enum island from, enum ticket t)
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

#define ISLAND_RXTMP_SIZE 0x1000
#define ISLAND_TXTMP_SIZE 0x1000

struct island_data {
	int cmd;
	int stop;
	char rxtmp[ISLAND_RXTMP_SIZE];
	char txtmp[ISLAND_TXTMP_SIZE];
	unsigned int rxlen;
	unsigned int txlen;
	unsigned int rxsize;
	unsigned int txsize;
	sem_t ready;
};

void island_data_init(struct island_data *idata)
{

}

void island_post_data(struct island_data *idata)
{
}

void *island_work(void *d)
{
	volatile int stop;
	enum island land;
	struct island_data *pd = d; 

	while (1) {
		sem_wait(pd->ready);
		stop = pd->stop;
		if (!stop) {
			switch (land) {
			case ISLAND_IDLE:
				break;
			case ISLAND_TX:
				break;
			case ISLAND_RX:
				break;
			default:
				break;
			}
		}
	}
}
