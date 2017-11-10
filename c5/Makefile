CC	:= gcc
INCS	:= include
TOPDIR	:= ./
CFLAGS	:= -I$(INCS) -L$(TOPDIR) -O2 -DDEBUG -g -Wall -Werror

APP	:= main
COBJS	:= main.o

all: $(APP)
	@echo "Build $(APP) success !"

$(APP): $(COBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(COBJS) $(APP)
