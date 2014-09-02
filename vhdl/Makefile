.PHONY : clean sim all

VLIB = work-obj93.cf
HOST := $(shell uname)
MACHINE := $(shell uname -m)
CHASSIS_SRCS = chassis1.wlist chassis2.wlist chassis5.wlist chassis11.wlist 
CHASSIS_V = $(CHASSIS_SRCS:.wlist=.vhd)
CABLE_SRCS = $(wildcard coax*.wlist)
VHD_SRCS = $(wildcard *.vhd)

FOREIGN = dtconn.o meminit.o \
	channel.o console.o cr405.o dd6603.o dd8xx.o mux6676.o \
	lp1612.o mt607.o mt669.o dcc6681.o \
	cr3447.o ddp.o niu.o lp3000.o cp3446.o \
	tpmux.o dtdisksubs.o charset.o dtnetsubs.o \
	trace.o log.o init.o device.o \
	dtmain.o deadstart.o
FLIBS   = -lpthread
VPATH = ..
LINKSWITCH := -Wl,

#ifeq ("$(HOST)","Darwin")
#CCOPTS := -arch i386
#ASMOPTS := -Wa,-arch -Wa,i386
#LDOPTS := -Wl,-arch -Wl,i386
CCOPTS := -m64
GOPTS := -Wc,$(CCOPTS)
#endif

OPTIMIZE ?=
#OPTIMIZE ?= -O2

GHDL ?= ghdl
GHDLOPTS := $(OPTIMIZE) $(GOPTS) $(ASMOPTS) $(LDOPTS) $(EXTRAGFLAGS)

all: cdc_tb

$(VLIB): $(VHD_SRCS) $(CHASSIS_V) cdc6600.vhd chassis12.vhd
	$(GHDL) -i *.vhd

cdc_tb:	$(VLIB) $(FOREIGN)
	$(GHDL) -m $(GHDLOPTS) $(addprefix $(LINKSWITCH),$(FOREIGN) $(FLIBS))  cdc_tb

cdc_tb2: $(VLIB) chassis12.vhd
	$(GHDL) -m $(GHDLOPTS) $(LINKSWITCH)$(FLIBS)  cdc_tb2

%.o : %.c
	$(CC) -O2  $(CFLAGS) $(CCOPTS) -I .. -I . -c $<

cdc6600.vhd $(CHASSIS_V) : $(CHASSIS_SRCS) $(CABLE_SRCS) wlist.py cmodule.py
	# Note: cables must be last
	python wlist.py  $(CHASSIS_SRCS) $(CABLE_SRCS)

chassis12.vhd : chassis12.wlist
	python wlist.py -t - chassis12.wlist

clean:
	rm -f *.o *.cf cyberdefs.stamp cdc6600.vhd chassis*.vhd $(VLIB)

sim: cdc_tb
	./cdc_tb --stop-time=10us --wave=cdc_tb.ghw

$(TOP): $(CHASSIS_O)

include $(CHASSIS_D)
