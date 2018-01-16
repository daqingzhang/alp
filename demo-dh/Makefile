CC	:= gcc
OBJDUMP	:= objdump
OBJCOPY	:= objcopy
TOPDIR	:= ./
INCS	:= -Iinclude
INCS	+= -I$(TOPDIR)
OUTDIR	:= out

GITVER	:= $(shell git log --dense --pretty=oneline -n1 | head -c8)

CFLAGS	:= $(INCS) -L$(TOPDIR) -O2 -g -Wall -Werror
CFLAGS	+= -D_REENTRANT
CFLAGS	+= -DDEBUG

APP	:= main-v$(GITVER)
COBJS	:= island.o oslib.o serial_thread.o
COBJS	+= comm_io.o comm_cmd.o comm_main.o
COBJS	+= sock_io.o sock_cmd.o server.o

all: $(APP)
	@echo "GITVER=$(GITVER)"
	@mkdir -p $(OUTDIR)
	mv *.o $(OUTDIR)
	@echo "Build $^ done !"

$(APP): $(COBJS)
	$(CC) $(CFLAGS) -o $@ $^  -lpthread

%.o:%.c
	@echo "CC $< --> $@"
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f  $(APP)
	rm -rf $(OUTDIR)
