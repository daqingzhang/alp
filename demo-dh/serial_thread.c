#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <serial_thread.h>
#include <island.h>

#define DBG printf

#define obj_to_prop(&(obj->prop))
#define obj_to_prop_conf(&(obj->prop.conf))
#define obj_to_prop_ctrl(&(obj->prop.ctrl))
#define obj_to_prop_rxchan(&(obj->prop.rxchan))
#define obj_to_prop_txchan(&(obj->prop.txchan))

static int serial_obj_cnt = 0;

/* Thread Functions */
static island_t serial_idle_process(struct serial_obj *obj, island_t land)
{
	struct serial_ctrl *ctrl = obj_to_prop_ctrl(obj);

	sem_wait(&ctrl->active);

	cmd  = ctrl->cmd;
	if (cmd == SER_CMD_TX) {
		land = take_ship(land, TK_IDLE_TO_TX);
	}
	if (cmd == SER_CMD_RX) {
		land = take_ship(land, TK_IDLE_TO_RX);
	}
	return land;
}

static island_t serial_tx_process(struct serial_obj *obj, island_t land)
{
	struct serial_ctrl *ctrl = obj_to_prop_ctrl(obj);

	sem_post(&ctrl->tx_ready);
	land = take_ship(land, TK_TX_TO_IDLE);

	return land;
}

static island_t serial_rx_process(struct serial_obj *obj, island_t land)
{
	struct serial_ctrl *ctrl = obj_to_prop_ctrl(obj);

	sem_post(&ctrl->rx_ready);
	land = take_ship(land, TK_RX_TO_IDLE);

	return land;
}

static void serial_run(void *data)
{
	struct serial_ctrl *ctrl = obj_to_prop_ctrl(obj);
	island_t land = ISLAND_IDLE;
	volatile int stop = 0;

	while(1) {
		switch (land) {
		case ISLAND_IDLE:
			land = serial_idle_process(obj, land);
			stop = ctrl->stop;
			break;
		case ISLAND_TX:
			land = serial_tx_process(obj, land);
			break;
		case ISLAND_RX:
			land = serial_rx_process(obj, land);
			break;
		}
		if (stop)
			break
	}
}

/* Private Functions */
static int serial_init(struct serial_obj *obj, const char *name)
{
	int r;
	struct serial_prop *prop = obj_to_prop(obj);
	struct serial_conf *conf = obj_to_prop_conf(obj);
	struct serial_ctrl *ctrl = obj_to_prop_ctrl(obj);

	// TODO: add lock
	prop->id = serial_obj_cnt++;
	prop->name = name;

	conf->name  = NULL;
	conf->speed = SER_CONF_DEFAULT_BAUD;
	conf->data  = SER_CONF_DEFAULT_DATA;
	conf->parity= SER_CONF_DEFAULT_PARITY;
	conf->stop  = SER_CONF_DEFAULT_STOP;

	ctrl->cmd   = 0;
	ctrl->open  = 0;
	ctrl->stop  = 0;
	sem_init(&ctrl->rx_ready, 0, 1);
	sem_init(&ctrl->tx_ready, 0, 1);
	sem_init(&ctrl->active, 0, 1);
	pthread_mutex_init(&ctrl->mutex, NULL);//none re-entrant
	pthread_attr_init(&ctrl->attr);
	ctrl->run = serial_run;

	r = pthread_create(&ctrl->pid, &ctrl->attr,
				ctrl->run, (void *)obj);
	if (r) {
		DBG("%s, create thread failed %d\n", __func__, r);
		return r;
	}
	DBG("%s, obj(%p) %s inited\n", __func__, obj, name);
	return 0;
}

static int serial_open(struct serial_obj *obj, const char *port)
{
}

static void serial_close(struct serial_obj *obj)
{
}

static unsigned int serial_read(struct serial_obj *obj, char *pbuf,
		unsigned int len, unsigned int timeout)
{
}

static unsigned int serial_write(struct serial_obj *obj, const char *pbuf,
			unsigned int len)
{
}

static void serial_debug(struct serial_obj *obj)
{
}

/* Public Functions */
struct serial_obj *serial_create(const char *name)
{
	int r;
	struct serial_obj *obj;

	obj = malloc(sizeof(struct serial_obj));
	if (!obj)
		return 0;

	obj->init  = serial_init;
	obj->open  = serial_open;
	obj->read  = serial_read;
	obj->write = serial_write;
	obj->close = serial_close;
	obj->debug = serial_debug;

	r = obj->init(obj, name);
	if (r) {
		DBG("%s, init obj failed %d\n", __func__, r);
		free(obj);
		return 0;
	}
	DBG("%s, obj(%p) created\n", __func__, obj);
	return obj;
}

static int serial_stop_thread(struct serial_obj *obj)
{
	int r;
	struct serial_ctrl *ctrl = obj_to_prop_ctrl(obj);

	ctrl->stop = 1;
	sem_post(&ctrl->active);

	r = pthread_join(ctrl->pid, NULL);
	if (r) {
		fprintf(stderr, "%s, join thread %x failed %d\n",
			__func__, ctrl->pid, r);
		return r;
	}
	return 0;
}

void serial_destroy(struct serial_obj *obj)
{
	if (!obj)
		return;

	if (obj->close)
		obj->close(obj);
	serial_stop_thread(obj);

	DBG("%s, obj(%p) deleted\n", __func__, obj);
	free(obj);
}
