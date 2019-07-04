#include <island.h>

static struct ship_ticket tickets[] = {
	{ ISLAND_IDLE,	ISLAND_IDLE,	TK_IDLE_TO_IDLE	},
	{ ISLAND_IDLE,	ISLAND_TX,	TK_IDLE_TO_TX	},
	{ ISLAND_TX,	ISLAND_IDLE,	TK_TX_TO_IDLE	},
	{ ISLAND_IDLE,	ISLAND_RX,	TK_IDLE_TO_RX	},
	{ ISLAND_RX,	ISLAND_RX,	TK_RX_TO_RX	},
	{ ISLAND_RX,	ISLAND_IDLE,	TK_RX_TO_IDLE	},
};

island_t take_ship(island_t from, enum ticket t)
{
	int i;
	island_t to = ISLAND_NULL;
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
