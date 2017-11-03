CC	:= gcc
INCS	:= include
CFLAGS	:= -I$(INCS) -O2 -DDEBUG -g --static

APP	:= main
COBJS	:= main.o abc.o

all: $(APP)
	@echo "done"

$(APP): $(COBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(COBJS) $(APP)
