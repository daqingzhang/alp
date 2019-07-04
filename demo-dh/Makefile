CC	:= gcc
CCP	:= g++
OBJDUMP	:= objdump
OBJCOPY	:= objcopy

ifeq ($(T),)
T	:= default
#$(warning "Please specify the target in command line: T=<Target>")
endif

ifneq ($(V),1)
ECHO :=@
else
ECHO :=
endif

obj-y	:=
sinclude config/$(T)/target.mk
sinclude config/$(T)/config.mk
sinclude common/config.mk
sinclude drivers/config.mk
sinclude socket/config.mk
sinclude tests/config.mk

COBJS	:=$(obj-y)
TOPDIR	:= $(shell pwd)
INCS	:= -Iinclude
INCS	+= -I$(TOPDIR) -Icommon -Idrivers -Itests -Isocket -Iconfig/$(T)
OUTDIR	:= out

GITVER	:= $(shell git log --dense --pretty=oneline -n1 | head -c8)
APP	:= main-v$(GITVER)

CFLAGS	:= $(INCS) -L$(TOPDIR) -O2 -g -Wall -Werror -Wl,-Map=$(APP).map
CFLAGS	+= -D_REENTRANT

ifeq ($(TEST),1)
CFLAGS	+= -DTEST
endif

ifeq ($(DEBUG),1)
CFLAGS	+= -DDEBUG
endif

.PHONY: all clean lst

all: clean $(APP)
	@mkdir -p $(OUTDIR)
#	@mv -f *.map $(OUTDIR)
	@echo "============================================"
	@echo "Build $^ done !"
	@echo "============================================"

$(APP): $(COBJS)
	$(ECHO)$(CCP) $(CFLAGS) -o $@ $^  -lpthread -lusb-1.0

lst: $(APP)
	$(ECHO)$(OBJDUMP) -d $< > $(APP).lst
	$(ECHO)$(OBJDUMP) -h $< > $(APP).sym
	$(ECHO)$(OBJDUMP) -t $< > $(APP).sec

%.o:%.cpp
	@echo "CCP $< --> $@"
	$(ECHO)$(CCP) $(CFLAGS) -c $< -o $@

%.o:%.c
	@echo "CC $< --> $@"
	$(ECHO)$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f *.o *.lst *.sym *.sec *.map *.out $(APP) $(COBJS) main-v*
	@rm -rf $(OUTDIR)
