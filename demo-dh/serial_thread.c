#include <island.h>
#include <serial_thread.h>

#define dev_to_prop(dev)	(&(dev->prop))
#define dev_to_prop_conf(dev)	(&(dev->prop.conf))
#define dev_to_prop_ctrl(dev)	(&(dev->prop.ctrl))
#define dev_to_prop_rxchan(dev)	(&(dev->prop.rxchan))
#define dev_to_prop_txchan(dev)	(&(dev->prop.txchan))

enum {
	SER_CMD_NULL,
	SER_CMD_RD,
	SER_CMD_WR,
};

static int serial_dev_cnt = 0;

#if 0
static int serial_lock(struct serial_dev *dev)
{
	pthread_mutex_lock(&dev->ctrl.mutex);
}

static int serial_unlock(struct serial_dev *dev)
{
	pthread_mutex_unlock(&dev->ctrl.mutex);
}
#endif

static void serial_send_cmd(struct serial_dev *dev, int cmd)
{
	struct serial_ctrl *ctrl = dev_to_prop_ctrl(dev);

	ctrl->cmd = cmd;
	os_sem_post(&ctrl->active);
}

static void serial_send_stop(struct serial_dev *dev, int stop)
{
	struct serial_ctrl *ctrl = dev_to_prop_ctrl(dev);

	ctrl->stop = stop;
	os_sem_post(&ctrl->active);
}

static int serial_wait_rx_ready(struct serial_dev *dev, unsigned int tm_sec)
{
	int r;
	struct serial_ctrl *ctrl = dev_to_prop_ctrl(dev);
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

	r = os_sem_timedwait(&ctrl->rx_ready, &ts);
	if (r)
		DBG("wait rx_ready error %d\n", r);
	return r;
}

static int serial_wait_tx_ready(struct serial_dev *dev, unsigned int tm)
{
	struct serial_ctrl *ctrl = dev_to_prop_ctrl(dev);
	int r;

	r = os_sem_wait(&ctrl->tx_ready);
	if (r)
		DBG("wait tx_ready error %d\n", r);
	return r;
}

static int serial_is_stopped(struct serial_dev *dev)
{
	struct serial_ctrl *ctrl = dev_to_prop_ctrl(dev);

	return ctrl->stop;
}

/* Thread Functions */
static island_t serial_idle_process(struct serial_dev *dev, island_t land)
{
	int cmd;
	struct serial_ctrl *ctrl = dev_to_prop_ctrl(dev);

	DBG("%s wait sem\n", __func__);

	os_sem_wait(&ctrl->active);

	DBG("%s wait sem done\n", __func__);

	if (serial_is_stopped(dev))
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

static island_t serial_tx_process(struct serial_dev *dev, island_t land)
{
	struct serial_ctrl *ctrl = dev_to_prop_ctrl(dev);

	DBG("%s tx done\n", __func__);
	land = take_ship(land, TK_TX_TO_IDLE);

#ifdef DEBUG
	usleep(100);
#endif

	// TODO: send data

	os_sem_post(&ctrl->tx_ready);

	return land;
}

static island_t serial_rx_process(struct serial_dev *dev, island_t land)
{
	int i, len;
	struct serial_ctrl *ctrl = dev_to_prop_ctrl(dev);
	struct serial_chan *rxchan = dev_to_prop_rxchan(dev);
	struct serial_chan *txchan = dev_to_prop_txchan(dev);
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

	os_sem_post(&ctrl->rx_ready);

	return land;
}

static island_t serial_rst_process(struct serial_dev *dev, island_t land)
{
	return ISLAND_IDLE;
}

static void* serial_run(void *data)
{
	struct serial_dev *dev = data;
	island_t land = ISLAND_IDLE;
	volatile int stop = 0;

	DBG("%s, start, data(%p)\n", __func__, data);
	while(1) {
		switch (land) {
		case ISLAND_IDLE:
			land = serial_idle_process(dev, land);
			if (serial_is_stopped(dev)) {
				stop = 1;
			}
			break;
		case ISLAND_TX:
			land = serial_tx_process(dev, land);
			break;
		case ISLAND_RX:
			land = serial_rx_process(dev, land);
			break;
		default:
			land = serial_rst_process(dev, land);
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
	return serial_dev_cnt++;
}

/* Private Functions */
static int serial_dev_init(struct serial_dev *dev, const char *name)
{
	int r;
	struct serial_prop *prop = dev_to_prop(dev);
	struct serial_conf *conf = dev_to_prop_conf(dev);
	struct serial_ctrl *ctrl = dev_to_prop_ctrl(dev);

	prop->id = serial_obtain_id();
	prop->name = name;

	conf->port  = NULL;
	conf->speed = SER_CONF_DEFAULT_BAUD;
	conf->data  = SER_CONF_DEFAULT_DATA;
	conf->parity= SER_CONF_DEFAULT_PARITY;
	conf->stop  = SER_CONF_DEFAULT_STOP;

	ctrl->cmd   = 0;
	ctrl->stop  = 0;
	ctrl->status= 0;
	ctrl->fd    = -1;
	ctrl->run   = serial_run;

	// init semaphore to zero
	r =  os_sem_init(&ctrl->rx_ready, 0, 0);
	r += os_sem_init(&ctrl->tx_ready, 0, 0);
	r += os_sem_init(&ctrl->active, 0, 0);
	if (r)
		return r;

	// init thread mutex, attr
	os_thread_mutex_init(&ctrl->mutex);
	os_thread_attr_init(&ctrl->attr);

	// create thread
	r = os_thread_create(&ctrl->pid, &ctrl->attr, ctrl->run, (void *)dev);
	if (r)
		return r;

	DBG("%s, dev(%p) %s inited\n", __func__, dev, name);
	return 0;
}

static int serial_dev_config(struct serial_dev *dev,
			int baud, int data, char parity, int stop)
{
	struct serial_conf *conf = dev_to_prop_conf(dev);

	conf->speed = baud;
	conf->data = data;
	conf->parity = parity;
	conf->stop = stop;

	// TODO: config serial
	return 0;
}

static int serial_dev_set_baud(struct serial_dev *dev, int baud)
{
	struct serial_conf *conf = dev_to_prop_conf(dev);

	conf->speed = baud;
	// TODO: config serial
	return 0;
}

static int serial_dev_open(struct serial_dev *dev, const char *port)
{
	struct serial_ctrl *ctrl = dev_to_prop_ctrl(dev);

	ctrl->status = 1;
	// TODO: config serial
	DBG("%s %s done\n", __func__, port);
	return 0;
}

static void serial_dev_close(struct serial_dev *dev)
{
	struct serial_ctrl *ctrl = dev_to_prop_ctrl(dev);

	if(ctrl->status) {
//		close(ctrl->fd);
		ctrl->status = 0;
		DBG("%s done\n", __func__);
	}
}

static int serial_dev_read(struct serial_dev *dev, char *pbuf,
		unsigned int len, unsigned int tm_sec)
{
	int r;
	int cnt = -1;
	struct serial_chan *rxchan = dev_to_prop_rxchan(dev);

	DBG("%s %d bytes\n", __func__, len);

	rxchan->cnt = 0;
	rxchan->len = len;
	rxchan->timeout = tm_sec;

	serial_send_cmd(dev, SER_CMD_RD);

	r = serial_wait_rx_ready(dev, tm_sec);
	if (!r) {
		cnt = (int)(rxchan->cnt);
		if (cnt)
			memcpy(pbuf, rxchan->buf, cnt);
	}
	return cnt;
}

static int serial_dev_write(struct serial_dev *dev, const char *pbuf,
			unsigned int len)
{
	int r;
	int cnt = -1;
	struct serial_chan *txchan = dev_to_prop_txchan(dev);

	DBG("%s %d bytes\n", __func__, len);

	txchan->cnt = 0;
	txchan->len = len;
	memcpy(txchan->buf, pbuf, len);

	serial_send_cmd(dev, SER_CMD_WR);
	r = serial_wait_tx_ready(dev, 0);
	if (!r)
		cnt = (int)(txchan->len);
	return cnt;
}

void serial_dev_debug(struct serial_dev *dev)
{
	DBG("%s\n", __func__);
}

struct serial_dev *serial_dev_create(const char *name)
{
	int r;
	struct serial_dev *dev;

	dev = malloc(sizeof(struct serial_dev));
	if (!dev)
		return 0;

	dev->open  = serial_dev_open;
	dev->read  = serial_dev_read;
	dev->write = serial_dev_write;
	dev->close = serial_dev_close;
	dev->config = serial_dev_config;
	dev->set_speed = serial_dev_set_baud;

	r = serial_dev_init(dev, name);
	if (r) {
		DBG("%s, init dev failed %d\n", __func__, r);
		free(dev);
		return 0;
	}
	DBG("%s, dev(%p) created\n", __func__, dev);
	return dev;
}

void serial_dev_destroy(struct serial_dev *dev)
{
	struct serial_ctrl *ctrl = dev_to_prop_ctrl(dev);
	int r;

	if (!dev)
		return;

	serial_send_stop(dev, 1);

	r = os_thread_destroy(ctrl->pid, NULL);
	if (r) {
		DBG("%s, join thread %d failed %d\n", __func__,
			(int)(ctrl->pid), r);
		return;
	}

	if (dev->close)
		dev->close(dev);

	DBG("%s, dev(%p) deleted\n", __func__, dev);
	free(dev);
}
