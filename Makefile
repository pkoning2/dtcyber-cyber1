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
LDFLAGS = -L/usr/X11R6/lib 
INCL    = -I/usr/X11R6/include -I/usr/local/include
#CFLAGS  = -O2 -I. $(INCL) -Wall -Wshadow -Wpointer-arith -Wmissing-prototypes 
CFLAGS  = -O2 -g2 -I. $(INCL)

CFLAGS += -DCcDebug=1

# If your host doesn't have pre-emptive threads (e.g., NetBSD)
# or you don't want to use multiple CPUs for the emulator,
# or the host doesn't have native 64-bit load/store (e.g., Mac G4)
# then comment out the next line:
CFLAGS += -DCPU_THREADS

# uncomment the next  two lines for Apple Mac X
#LIBS    +=  /System/Library/Frameworks/Carbon.framework/Carbon
#INCL    += -I/System/Library/Frameworks/Carbon.framework/Headers

# uncomment the next line for Apple Mac on a G5 (but NOT on other CPUs!)
#CFLAGS += -mcpu=970 -mtune=970 -falign-loops=16 -falign-functions=16 -falign-labels=16 -mpowerpc64

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

dtcyber: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $+ $(LIBS)

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
