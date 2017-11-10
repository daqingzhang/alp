CC	:= gcc
INCS	:= include
TOPDIR	:= ./
CFLAGS	:= -I$(INCS) -L$(TOPDIR) -O2 -DDEBUG -g -Wall -Werror

SERVER	:= server
CLIENT	:= client
SER_COBJS	:= server.o
CLI_COBJS	:= client.o

all: $(SERVER) $(CLIENT)
	@echo "Build $(SERVER) success !"

$(CLIENT): $(CLI_COBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(SERVER): $(SER_COBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SER_COBJS) $(CLI_COBJS) $(SERVER) $(CLIENT)
