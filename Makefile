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

HOST := $(shell uname)

LIBS    = -lm -lX11 -lpthread
LDFLAGS = -g2 -L/usr/X11R6/lib
INCL    = -I/usr/X11R6/include -I/usr/local/include

ifeq ("$(HOST)","Darwin")
LIBS    +=  /System/Library/Frameworks/Carbon.framework/Carbon
INCL    += -I/System/Library/Frameworks/Carbon.framework/Headers
G5CFLAGS = -mcpu=G5 -mtune=G5 -falign-loops=16 -falign-functions=16 -falign-labels=16 -mpowerpc64 -DCPU_THREADS
G4CFLAGS = -mcpu=G4 -mtune=G4
G3CFLAGS = -mcpu=G3 -mtune=G3
else
LDFLAGS +=  -L/usr/local/lib
endif

CDEBUG = -DCcDebug=1

CFLAGS  = -O2 -g2 -I. $(INCL) $(CDEBUG) $(EXTRACFLAGS)

PCFS	= seymour8b.pcf seymour8m.pcf \
	seymour16b.pcf seymour16m.pcf \
	seymour32b.pcf seymour32m.pcf \
	plato.pcf

SOBJS	= pterm_x11.o ptermcom.o charset.o

OBJS    = main.o window_x11.o init.o trace.o dump.o \
          device.o channel.o cpu.o pp.o float.o shift.o operator.o \
          deadstart.o console.o cr405.o dd6603.o dd844.o mux6676.o \
          lp1612.o mt607.o mt669.o dcc6681.o rtc.o \
	  cr3447.o ddp.o niu.o lp3000.o cp3446.o \
	  $(SOBJS)

all: dtcyber fonts pterm

fonts: $(PCFS)

ifeq ("$(HOST)","Darwin")
dtcyber:
	mkdir -p g3; \
	cd g3; \
	$(MAKE) -f ../Makefile gxdtcyber EXTRACFLAGS="$(G3CFLAGS)" VPATH=..
#	mkdir -p g4; \
#	cd g4; \
#	$(MAKE) -f ../Makefile gxdtcyber EXTRACFLAGS="$(G4CFLAGS)" VPATH=..
	mkdir -p g5; \
	cd g5; \
	$(MAKE) -f ../Makefile gxdtcyber EXTRACFLAGS="$(G5CFLAGS)" VPATH=..
	lipo -create -output dtcyber g3/gxdtcyber g5/gxdtcyber

gxdtcyber: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $+ $(LIBS)

clean:
	rm -f *.o *.pcf g3/*.o g3/gxdcyber g4/*.o g4/gxdcyber g5/*.o g5/gxdtcyber
else
dtcyber: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $+ $(LIBS)

clean:
	rm -f *.o *.pcf dtcyber
endif

pterm:	$(SOBJS) pterm.o
	$(CC) $(LDFLAGS) -o $@ $+ $(LIBS)

buildall: clean all

%.o : %.c
	$(CC) $(CFLAGS) -c $<

%.pcf : %.bdf
	bdftopcf -o $@ $<

#---------------------------  End Of File  --------------------------------
