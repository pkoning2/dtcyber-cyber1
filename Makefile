#--------------------------------------------------------------------------
#
#   Copyright (c) 2002, Tom Hunter (see license.txt)
#
#   Name: Makefile.x11
#
#   Description:
#       Build X11 version of Desktop Cyber emulation.
#
#--------------------------------------------------------------------------

LIBS    = -lm -lX11 -lpthread
LIBS    +=  /System/Library/Frameworks/Carbon.framework/Carbon
#LIBS    = -lm -lX11
#LDFLAGS = -s -L/usr/X11R6/lib
LDFLAGS = -L/usr/X11R6/lib 
#LDFLAGS += -L/usr/local/lib
INCL    = -I/usr/X11R6/include -I/usr/local/include -I/System/Library/Frameworks/Carbon.framework/Headers

#CFLAGS  = -O2 -I. $(INCL) -Wall -Wshadow -Wpointer-arith -Wmissing-prototypes 
CFLAGS  = -O2 -g2 -I. $(INCL)
CFLAGS += -mcpu=970 -mtune=970 -falign-loops=16 -falign-functions=16 -falign-labels=16 -mpowerpc64
CFLAGS += -DCcDebug=1
CFLAGS += -DCPU_THREADS
VPATH = /Users/koning/dtcyber

PCFS	= seymour8b.pcf seymour8m.pcf \
	seymour16b.pcf seymour16m.pcf \
	seymour32b.pcf seymour32m.pcf \
	plato.pcf

SOBJS	= pterm_x11.o ptermcom.o charset.o

OBJS    = main.o window_x11.o init.o trace.o dump.o \
          device.o channel.o cpu.o pp.o float.o shift.o operator.o \
          deadstart.o console.o cr405.o dd6603.o dd844.o mux6676.o \
          lp1612.o mt607.o mt669.o dcc6681.o rtc.o \
	  cr3447.o ddp.o niu.o lp3000.o $(SOBJS)

all: dtcyber fonts pterm

fonts: $(PCFS)

dtcyber: g4/dtcyber g5/dtcyber
	lipo -create -output dtcyber g4/dtcyber g5/dtcyber

g4/dtcyber:
	cd g4; $(MAKE) dtcyber

g5/dtcyber:
	cd g5; $(MAKE) dtcyber

pterm:	$(SOBJS) pterm.o
	$(CC) $(LDFLAGS) -o $@ $+ $(LIBS)

buildall: clean all

clean:
	rm -f *.o *.pcf

%.o : %.c
	$(CC) $(CFLAGS) -c $<

%.pcf : %.bdf
	bdftopcf -o $@ $<

#---------------------------  End Of File  --------------------------------
