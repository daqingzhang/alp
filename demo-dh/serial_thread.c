#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <serial_thread.h>
#include <island.h>

#define DBG printf

#define obj_to_prop(obj)	(&(obj->prop))
#define obj_to_prop_conf(obj)	(&(obj->prop.conf))
#define obj_to_prop_ctrl(obj)	(&(obj->prop.ctrl))
#define obj_to_prop_rxchan(obj)	(&(obj->prop.rxchan))
#define obj_to_prop_txchan(obj)	(&(obj->prop.txchan))

enum {
	SER_CMD_NULL,
	SER_CMD_RD,
	SER_CMD_WR,
};

static int serial_obj_cnt = 0;

#if 0
static int serial_lock(struct serial_obj *obj)
{
	pthread_mutex_lock(&obj->ctrl.mutex);
}

static int serial_unlock(struct serial_obj *obj)
{
	pthread_mutex_unlock(&obj->ctrl.mutex);
}
#endif

static void serial_send_cmd(struct serial_obj *obj, int cmd)
{
	struct serial_ctrl *ctrl = obj_to_prop_ctrl(obj);

	ctrl->cmd = cmd;
	sem_post(&ctrl->active);
}

static void serial_send_stop(struct serial_obj *obj, int stop)
{
	struct serial_ctrl *ctrl = obj_to_prop_ctrl(obj);

	ctrl->stop = stop;
	sem_post(&ctrl->active);
}

static int serial_wait_rx_ready(struct serial_obj *obj, unsigned int tm_sec)
{
	int r;
	struct serial_ctrl *ctrl = obj_to_prop_ctrl(obj);
	struct timespec ts;

	r = clock_gettime(CLOCK_REALTIME, &ts);
	if (r) {
		printf("get time failed %d\n", r);
		return r;
	}

	tm_sec = (tm_sec == 0) ? 1 : tm_sec;
	ts.tv_sec += tm_sec;
	DBG("%s, tv_sec=%d, tm_sec=%d\n", __func__,
		(unsigned int)(ts.tv_sec), tm_sec);

	r = sem_timedwait(&ctrl->rx_ready, &ts);
	if (r)
		DBG("wait rx_ready error %d\n", r);
	return r;
}

static int serial_wait_tx_ready(struct serial_obj *obj, unsigned int tm)
{
	struct serial_ctrl *ctrl = obj_to_prop_ctrl(obj);
	int r;

	r = sem_wait(&ctrl->tx_ready);
	if (r)
		DBG("wait tx_ready error %d\n", r);
	return r;
}

static int serial_is_stopped(struct serial_obj *obj)
{
	struct serial_ctrl *ctrl = obj_to_prop_ctrl(obj);

	return ctrl->stop;
}

/* Thread Functions */
static island_t serial_idle_process(struct serial_obj *obj, island_t land)
{
	int cmd;
	struct serial_ctrl *ctrl = obj_to_prop_ctrl(obj);

	DBG("%s wait sem\n", __func__);

	sem_wait(&ctrl->active);

	DBG("%s wait sem done\n", __func__);

	if (serial_is_stopped(obj))
		return land;

	cmd  = ctrl->cmd;
	ctrl->cmd = SER_CMD_NULL;

	if (cmd == SER_CMD_WR) {
		land = take_ship(land, TK_IDLE_TO_TX);
	} else if (cmd == SER_CMD_RD) {
		land = take_ship(land, TK_IDLE_TO_RX);
	}
	return land;
}

static island_t serial_tx_process(struct serial_obj *obj, island_t land)
{
	struct serial_ctrl *ctrl = obj_to_prop_ctrl(obj);

	DBG("%s tx done\n", __func__);
	land = take_ship(land, TK_TX_TO_IDLE);

#ifdef DEBUG
	usleep(100);
#endif

	// TODO: send data

	sem_post(&ctrl->tx_ready);

	return land;
}

static island_t serial_rx_process(struct serial_obj *obj, island_t land)
{
	int i, len;
	struct serial_ctrl *ctrl = obj_to_prop_ctrl(obj);
	struct serial_chan *rxchan = obj_to_prop_rxchan(obj);
	struct serial_chan *txchan = obj_to_prop_txchan(obj);
	char *rxbuf = rxchan->buf;

	DBG("%s rx done\n", __func__);
	land = take_ship(land, TK_RX_TO_IDLE);

#ifdef DEBUG
	len = txchan->len;
	for(i = 0;i < len; i++) {
		rxbuf[i] = txchan->buf[i];
	}
	rxchan->cnt = i;
	usleep(100);
#endif
	// TODO: receive data

	sem_post(&ctrl->rx_ready);

	return land;
}

static island_t serial_rst_process(struct serial_obj *obj, island_t land)
{
	return ISLAND_IDLE;
}

static void* serial_run(void *data)
{
	struct serial_obj *obj = data;
	island_t land = ISLAND_IDLE;
	volatile int stop = 0;

	DBG("%s, start, data(%p)\n", __func__, data);
	while(1) {
		switch (land) {
		case ISLAND_IDLE:
			land = serial_idle_process(obj, land);
			if (serial_is_stopped(obj)) {
				stop = 1;
			}
			break;
		case ISLAND_TX:
			land = serial_tx_process(obj, land);
			break;
		case ISLAND_RX:
			land = serial_rx_process(obj, land);
			break;
		default:
			land = serial_rst_process(obj, land);
			break;
		}
		if (stop)
			break;
	}
	DBG("%s, end\n", __func__);
	return 0;
}

static int serial_obtain_id(void)
{
	// TODO: lock counter
	return serial_obj_cnt++;
}

/* Private Functions */
static int serial_init(struct serial_obj *obj, const char *name)
{
	int r;
	struct serial_prop *prop = obj_to_prop(obj);
	struct serial_conf *conf = obj_to_prop_conf(obj);
	struct serial_ctrl *ctrl = obj_to_prop_ctrl(obj);

	prop->id = serial_obtain_id();
	prop->name = name;

	conf->name  = NULL;
	conf->speed = SER_CONF_DEFAULT_BAUD;
	conf->data  = SER_CONF_DEFAULT_DATA;
	conf->parity= SER_CONF_DEFAULT_PARITY;
	conf->stop  = SER_CONF_DEFAULT_STOP;

	ctrl->cmd   = 0;
	ctrl->status= 0;
	ctrl->stop  = 0;
	ctrl->fd    = -1;
	r =  sem_init(&ctrl->rx_ready, 0, 0);
	r += sem_init(&ctrl->tx_ready, 0, 0);
	r += sem_init(&ctrl->active, 0, 0);
	if (r) {
		DBG("%s, init sem failed %d\n", __func__, r);
		return r;
	}
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

static int serial_config(struct serial_obj *obj, int databit, char parity,
				int stopbit)
{
	struct serial_conf *conf = obj_to_prop_conf(obj);

	conf->data = databit;
	conf->parity = parity;
	conf->stop = stopbit;

	// TODO: config serial
	return 0;
}

static int serial_set_speed(struct serial_obj *obj, int baud)
{
	struct serial_conf *conf = obj_to_prop_conf(obj);

	conf->speed = baud;
	// TODO: config serial
	return 0;
}

static int serial_open(struct serial_obj *obj, const char *port)
{
	struct serial_ctrl *ctrl = obj_to_prop_ctrl(obj);

	ctrl->status = 1;
	DBG("%s %s done\n", __func__, port);
	return 0;
}

static void serial_close(struct serial_obj *obj)
{
	struct serial_ctrl *ctrl = obj_to_prop_ctrl(obj);

	if(ctrl->status) {
//		close(ctrl->fd);
		ctrl->status = 0;
		DBG("%s done\n", __func__);
	}
}

static int serial_read(struct serial_obj *obj, char *pbuf,
		unsigned int len, unsigned int tm_sec)
{
	int r;
	int cnt = -1;
	struct serial_chan *rxchan = obj_to_prop_rxchan(obj);

	DBG("%s %d bytes\n", __func__, len);

	rxchan->cnt = 0;
	rxchan->len = len;
	rxchan->timeout = tm_sec;

	serial_send_cmd(obj, SER_CMD_RD);

	r = serial_wait_rx_ready(obj, tm_sec);
	if (!r) {
		cnt = (int)(rxchan->cnt);
		if (cnt)
			memcpy(pbuf, rxchan->buf, cnt);
	}
	return cnt;
}

static int serial_write(struct serial_obj *obj, const char *pbuf,
			unsigned int len)
{
	int r;
	int cnt = -1;
	struct serial_chan *txchan = obj_to_prop_txchan(obj);

	DBG("%s %d bytes\n", __func__, len);

	txchan->cnt = 0;
	txchan->len = len;
	memcpy(txchan->buf, pbuf, len);

	serial_send_cmd(obj, SER_CMD_WR);
	r = serial_wait_tx_ready(obj, 0);
	if (!r)
		cnt = (int)(txchan->len);
	return cnt;
}

void serial_debug(struct serial_obj *obj)
{
	DBG("%s\n", __func__);
}

/* Public Functions */
struct serial_obj *serial_create(const char *name)
{
	int r;
	struct serial_obj *obj;

	obj = malloc(sizeof(struct serial_obj));
	if (!obj)
		return 0;

	obj->open  = serial_open;
	obj->read  = serial_read;
	obj->write = serial_write;
	obj->close = serial_close;
	obj->config = serial_config;
	obj->set_speed = serial_set_speed;

	r = serial_init(obj, name);
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

	serial_send_stop(obj, 1);

	r = pthread_join(ctrl->pid, NULL);
	if (r) {
		fprintf(stderr, "%s, join thread %x failed %d\n",
			__func__, (int)(ctrl->pid), r);
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
