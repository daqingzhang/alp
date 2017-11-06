CC	:= gcc
AR	:= ar
INCS	:= include
TOPDIR	:= ./
CFLAGS	:= -I$(INCS) -L$(TOPDIR) -O2 -DDEBUG -g -Wall --static

APP	:= main
COBJS	:= main.o

SLIB	:= libtest.a
SLOBJS	:= a.o b.o

DLIB	:= libtest2.a
DLSRCS	:= c.c d.c
DLOBJS	:= c.o d.o

all: $(SLIB) $(APP)
	@echo "done"

$(APP): $(COBJS)
	$(CC) $(CFLAGS) -o $@ $^ -ltest

$(SLIB): $(SLOBJS)
	$(AR) cr $@ $^

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(COBJS) $(APP) $(SLIB) $(SLOBJS)
