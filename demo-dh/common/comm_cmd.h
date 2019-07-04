#ifndef __COMM_CMD_H__
#define __COMM_CMD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <debug.h>
#include <string.h>

#define COMM_TEMP_BUF_SIZE 4096
#define COMM_ARGV_BUF_SIZE 2048

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
	char *name;
	char *desc;
	void *priv;
	struct comm_data *cdata;
	void (*handler)(int id, void *comm_data, void *priv);
};

/*
 * common io function
 ******************************************************
 */
int comm_fgetc(void);
int comm_puts(const char *s);
int user_cmd_register(void);
int user_cmd_unregister(void);

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

/*
 * common entry function
 ******************************************************
 */
int comm_main(int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

#endif
