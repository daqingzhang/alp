CC	:= gcc
OBJDUMP	:= objdump
OBJCOPY	:= objcopy
TOPDIR	:= ./
INCS	:= -Iinclude
INCS	+= -I$(TOPDIR)

CFLAGS	:= $(INCS) -L$(TOPDIR) -O2 -g -Wall -Werror
CFLAGS	+= -D_REENTRANT
CFLAGS	+= -DDEBUG

APP	:= main
COBJS	:= island.o oslib.o serial_thread.o
COBJS	+= comm_io.o comm_cmd.o comm_main.o
COBJS	+= sock_io.o sock_cmd.o server.o

all: $(APP)
	@echo "Build $^ done !"

$(APP): $(COBJS)
	$(CC) $(CFLAGS) -o $@ $^  -lpthread

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(COBJS) $(APP)
