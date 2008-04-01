VLIB = work-obj93.cf
MODULES = pd.o pg.o pr.o tc.o td.o th.o tg.o pk.o clock.o
CHASSIS = chassis1.o
TOP = cdc6600.o
TB = cdc_tb.o
OBJS = $(TB) $(TOP) $(MODULES)
cdc_tb:	$(OBJS) $(VLIB)
	ghdl -e cdc_tb

cyberdefs.o cyberdefs.stamp: cyberdefs.vhd
	ghdl -a $<
	touch cyberdefs.stamp

%.o : %.vhdl cyberdefs.stamp
	ghdl -a $<

%.o : %.vhd cyberdefs.stamp
	ghdl -a $<

$(VLIB): cyberdefs.vhd
	ghdl -a $<

clean:
	rm -f *.o *.cf cyberdefs.stamp

$(TOP): $(CHASSIS)

$(CHASSIS): $(MODULES)
