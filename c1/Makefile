CC	:= gcc
INCS	:= include
CFLAGS	:= -I$(INCS) -O0 -DDEBUG -g --static

APP	:= main
COBJS	:= main.o abc.o
#CSRCS	:= main.c abc.c

all: $(APP)
	@echo "done"

$(APP): $(COBJS)
	$(CC) $(CFLAGS) -o $@ $^

#$(COBJS): $(CSRCS)
#	$(CC) $(CFLAGS) -c $^

%.o:%.c
	$(CC) $(CFLAGS) -c $^

clean:
	rm -f $(COBJS) $(APP)
