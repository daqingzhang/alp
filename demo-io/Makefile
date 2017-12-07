CC	:= gcc
TOPDIR	:= ./
INCS	:= -Iinclude
INCS	+= -I$(TOPDIR)

CFLAGS	:= $(INCS) -L$(TOPDIR) -O2 -g -Wall
CFLAGS	+= -DDEBUG

APP	:= main
COBJS	:= comm_file.o main.o

all: $(APP)
	@echo "Build $^ done !"

$(APP): $(COBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(COBJS) $(APP)
