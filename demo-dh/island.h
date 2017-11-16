#ifndef __ISLAND_H__
#define __ISLAND_H__

#define ARRAY_SIZE(a) (sizeof((a)) / sizeof((a)[0]))

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

typedef enum {
	ISLAND_NULL,
	ISLAND_IDLE,
	ISLAND_TX,
	ISLAND_RX,
}island_t;

struct ship_ticket {
	island_t from;
	island_t to;
	enum ticket t;
};

island_t take_ship(island_t from, enum ticket t);

#endif
