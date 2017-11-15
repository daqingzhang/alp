CC	:= gcc
OBJDUMP	:= objdump
OBJCOPY	:= objcopy
TOPDIR	:= ./
INCS	:= -Iinclude
INCS	+= -I$(TOPDIR)

CFLAGS	:= $(INCS) -L$(TOPDIR) -O2 -g -Wall
#CFLAGS	+= -DDEBUG

APP	:= box
COBJS	:= comm_cmd.o calc.o box.o

all: $(APP)
	@echo "Build $^ done !"

$(APP): $(COBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(COBJS) $(APP)
