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
ccflags-y	:=
sinclude config/$(T)/target.mk
sinclude config/$(T)/config.mk
sinclude common/config.mk
sinclude drivers/config.mk
sinclude socket/config.mk
sinclude tests/config.mk

COBJS	:=$(obj-y)
DEPS	:=$(COBJS:.o=.o.d)

TOPDIR	:= $(shell pwd)
INCS	:= -Iinclude
INCS	+= -I$(TOPDIR) -Icommon -Idrivers -Itests -Isocket -Iconfig/$(T)
OUTDIR	:= out/$(T)

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

ifeq ($(USE_SOCKET),1)
CFLAGS	+= -DUSE_SOCKET
endif

ifeq ($(CMD_MODULE_USB),1)
CFLAGS	+= -DCMD_MODULE_USB
endif

ifeq ($(CMD_MODULE_SOCK),1)
CFLAGS	+= -DCMD_MODULE_SOCK
endif

ifneq ($(ccflags-y),)
CFLAGS	+= $(ccflags-y)
endif

.PHONY: all clean lst

all: $(APP)
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
	@echo "C++	$@"
	$(ECHO)$(CCP) $(CFLAGS) -MD -MF $@.d -MP -c -o $@ $<

%.o:%.c
	@echo "CC	$@"
	$(ECHO)$(CC) $(CFLAGS) -MD -MF $@.d -MP -c -o $@ $<

odir:
	@mkdir -p $(OUTDIR)/socket
	@mkdir -p $(OUTDIR)/tests
	@mkdir -p $(OUTDIR)/drivers
	@mkdir -p $(OUTDIR)/common

mvobj: odir
	@mv socket/*.o $(OUTDIR)/socket/
	@mv tests/*.o $(OUTDIR)/tests/
	@mv drivers/*.o $(OUTDIR)/drivers/
	@mv common/*.o $(OUTDIR)/common/

clean:
	@rm -f *.o *.lst *.sym *.sec *.map *.out $(APP) \
		$(DEPS) $(COBJS) main-v*
	@rm -rf $(OUTDIR)
