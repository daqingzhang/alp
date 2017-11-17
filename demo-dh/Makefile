CC	:= gcc
OBJDUMP	:= objdump
OBJCOPY	:= objcopy
TOPDIR	:= ./
INCS	:= -Iinclude
INCS	+= -I$(TOPDIR)

CFLAGS	:= $(INCS) -L$(TOPDIR) -O2 -g -Wall
CFLAGS	+= -D_REENTRANT
CFLAGS	+= -DDEBUG

APP	:= main
COBJS	:= island.o serial_thread.o main.o

all: $(APP)
	@echo "Build $^ done !"

$(APP): $(COBJS)
	$(CC) $(CFLAGS) -o $@ $^  -lpthread

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(COBJS) $(APP)
