CC	:= gcc
INC	:= ./
CFLAGS	:= -I$(INC) -O0 -g -Wall -Werror -DDEBUG

APP	:= main
COBJS	:= main.o

all: $(APP)
	@echo "build $@ done"

$(APP): $(COBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(COBJS) $(APP)
