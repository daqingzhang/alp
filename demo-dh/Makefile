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
COBJS	:= island.o oslib.o serial_dev.o
COBJS	+= comm_cmd.o dh_sock.o server.o

all: $(APP)
	@echo "Build $^ done !"

$(APP): $(COBJS)
	$(CC) $(CFLAGS) -o $@ $^  -lpthread

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(COBJS) $(APP)
