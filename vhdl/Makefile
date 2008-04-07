VLIB = work-obj93.cf
MODULES = pd.o pg.o pr.o tc.o td.o th.o tg.o pk.o \
	clock.o mem.o  synchro.o
FOREIGN = dtsynchro.o \
	channel.o console.o cr405.o dd6603.o dd8xx.o mux6676.o \
	lp1612.o mt607.o mt669.o dcc6681.o \
	cr3447.o ddp.o niu.o lp3000.o cp3446.o \
	tpmux.o dtdisksubs.o charset.o dtnetsubs.o \
	trace.o log.o init.o device.o \
	dtmain.o
FLIBS   = -lpthread
CHASSIS = chassis1.o
TOP = cdc6600.o
TB = cdc_tb.o
OBJS = $(TB) $(TOP) $(MODULES) $(FOREIGN)
VPATH = ..
LINKSWITCH := -Wl,
LINK_FOREIGN = $(addprefix $(LINKSWITCH),$(FOREIGN) $(FLIBS))

cdc_tb:	$(OBJS) $(VLIB)
	ghdl -e $(LINK_FOREIGN)  cdc_tb
cyberdefs.o cyberdefs.stamp: cyberdefs.vhd
	ghdl -a $<
	touch cyberdefs.stamp

%.o : %.vhd cyberdefs.stamp
	ghdl -a $<

%.o : %.c
	$(CC) -O2  $(CFLAGS) -I .. -I . -c $<

$(VLIB): cyberdefs.vhd
	ghdl -a $<

clean:
	rm -f *.o *.cf cyberdefs.stamp

$(TOP): $(CHASSIS)

$(CHASSIS): $(MODULES)
