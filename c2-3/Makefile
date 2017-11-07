CC	:= gcc
AR	:= ar
INCS	:= include
TOPDIR	:= $(shell pwd)
CFLAGS	:= -I$(INCS) -L$(TOPDIR) -O2 -DDEBUG -g -Wall
LFLAGS	:= -fPIC

APP	:= main
COBJS	:= main.o
CSRCS	:= $(COBJS:.o=.c)

LIB	:= libtest.so
LOBJS	:= a.o b.o
LSRCS	:= $(LOBJS:.o=.c)

all: $(LIB) $(APP)
	@echo "done"

$(APP): $(COBJS)
	$(CC) $(CFLAGS) -o $@ $^ -ltest -Wl,-rpath,$(TOPDIR)

$(COBJS):$(CSRCS)
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB): $(LOBJS)
	$(CC) $(LFLAGS) -shared -o $@ $^

$(LOBJS): $(LSRCS)
	$(CC) $(CFLAGS) $(LFLAGS) -c $^

clean:
	rm -f $(COBJS) $(APP) $(LIB) $(LOBJS)
