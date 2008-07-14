VLIB = work-obj93.cf
MODULE_SRCS = $(wildcard ??.vhd)
MODULES = clock.o clockh.o mem.o  synchro.o $(MODULE_SRCS:.vhd=.o)
#CHASSIS_SRCS = $(wildcard chassis*.wlist)
# omit chassis 12 for now:
CHASSIS_SRCS = chassis1.wlist chassis11.wlist
CHASSIS_V = $(CHASSIS_SRCS:.wlist=.vhd)
CHASSIS = $(CHASSIS_SRCS:.wlist=.o)

FOREIGN = dtsynchro.o \
	channel.o console.o cr405.o dd6603.o dd8xx.o mux6676.o \
	lp1612.o mt607.o mt669.o dcc6681.o \
	cr3447.o ddp.o niu.o lp3000.o cp3446.o \
	tpmux.o dtdisksubs.o charset.o dtnetsubs.o \
	trace.o log.o init.o device.o \
	dtmain.o deadstart.o
FLIBS   = -lpthread
TOP = cdc6600.o
TB = cdc_tb.o
VOBJS = $(TB) $(TOP) $(MODULES)
OBJS = $(VOBJS) $(FOREIGN)
VPATH = ..
LINKSWITCH := -Wl,

OPTIMIZE ?=
#OPTIMIZE ?= -O2

cdc_tb:	$(OBJS)
	ghdl -e $(addprefix $(LINKSWITCH),$(filter-out $(VOBJS),$+) $(FLIBS))  cdc_tb

cdc_tb2:	$(filter-out cdc_tb.o,$(OBJS)) cdc_tb2.o chassis12.o
	ghdl -e $(addprefix $(LINKSWITCH),$(filter-out cdc_tb2.o chassis12.o $(VOBJS),$+) $(FLIBS))  cdc_tb2

cyberdefs.o cyberdefs.stamp: cyberdefs.vhd
	ghdl -a $<
	touch cyberdefs.stamp

%.o : %.vhd cyberdefs.stamp
	ghdl -a $(OPTIMIZE) $<

%.o : %.c
	$(CC) -O2  $(CFLAGS) -I .. -I . -c $<

cdc6600.vhd ${CHASSIS_V} : ${CHASSIS_SRCS}
	python wirelist.py ${CHASSIS_SRCS}

chassis12.vhd : chassis12.wlist
	python wirelist.py -t - chassis12.wlist

$(VLIB): cyberdefs.vhd
	ghdl -a $<

clean:
	rm -f *.o *.cf cyberdefs.stamp

$(TOP): $(CHASSIS)

$(CHASSIS): $(MODULES)
