#ifndef __COMM_CMD_H__
#define __COMM_CMD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <debug.h>
#include <string.h>

#define COMM_TEMP_BUF_SIZE 4096
#define COMM_ARGV_BUF_SIZE 2048

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof((a)) / sizeof((a)[0]))
#endif

/*
 * command data structure
 ******************************************************
 */
struct comm_data {
	char	temp[COMM_TEMP_BUF_SIZE];
	char*	argv[COMM_ARGV_BUF_SIZE];
	int	argc;
	int	tmpc;
	int 	tmpsize;
	int 	argsize;
};

/*
 * common command structure
 ******************************************************
 */
struct comm_cmd {
	int id;
	const char *name;
	const char *group;
	const char *desc;
	void *priv;
	struct comm_data *cdata;
	void (*handler)(int id, void *comm_data, void *priv);
	int nogroup;
};

typedef int (*comm_func_t)(void);

struct comm_cmd_module {
	comm_func_t init;
	comm_func_t exit;
};

#define CMD_MODULE(_init, _exit) \
	{ \
		.init = _init, \
		.exit = _exit, \
	}

/*
 * common io function
 ******************************************************
 */
int comm_fgetc(void);
int comm_puts(const char *s);

/*
 * common command driver
 ******************************************************
 */
void comm_clear_screen(void);
void comm_show_screen(void);
void comm_show_command(void);
void comm_init_command(void);
int comm_data_init(struct comm_data *d);
int comm_get_command(struct comm_data *d);
int comm_parse_command(struct comm_data *d);
int comm_exec_command(struct comm_data *d);
int comm_cmd_register(struct comm_cmd *cmd);
int comm_cmd_unregister(struct comm_cmd *cmd);
int comm_user_cmd_register(void);
int comm_user_cmd_unregister(void);

/*
 * common entry function
 ******************************************************
 */
int comm_main(int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

#endif
