CC	:= gcc
INCS	:= include
TOPDIR	:= ./
CFLAGS	:= -I$(INCS) -L$(TOPDIR) -O2 -DDEBUG -g -Wall

APP	:= main
COBJS	:= main.o

all: $(APP)
	@echo "done"

$(APP): $(COBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(COBJS) $(APP)
