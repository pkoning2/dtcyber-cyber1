#--------------------------------------------------------------------------
#
#   Copyright (c) 2002-2005, Tom Hunter, Paul Koning (see license.txt)
#
#   Name: Makefile
#
#   Description:
#       Build Desktop Cyber emulation on Unix-like systems.
#
#--------------------------------------------------------------------------

all: dtcyber pterm dtoper dd60 blackbox

include Makefile.common

include Makefile.wxpterm

include Makefile.dtoper

include Makefile.dd60

ifeq ("$(HOST)","Darwin")
#SDKDIR	?= /Developer/SDKs/MacOSX10.2.8.sdk
SDKDIR	?= /Developer/SDKs/MacOSX10.3.0.sdk
LIBS    +=  $(SDKDIR)/System/Library/Frameworks/Carbon.framework/Carbon
INCL    += -I$(SDKDIR)/System/Library/Frameworks/Carbon.framework/Headers
G5CFLAGS = -mcpu=G5 -mtune=G5 -falign-loops=16 -falign-functions=16 -falign-labels=16 -mpowerpc64
G4CFLAGS = -mcpu=G4 -mtune=G4
G3CFLAGS = -mcpu=G3 -mtune=G3
endif

CDEBUG = -DCcDebug=1

OBJS    = main.o init.o trace.o dump.o \
          device.o channel.o cpu.o pp.o float.o shift.o operator.o \
          deadstart.o console.o cr405.o dd6603.o dd8xx.o mux6676.o \
          lp1612.o mt607.o mt669.o dcc6681.o rtc.o log.o \
	  cr3447.o ddp.o niu.o lp3000.o cp3446.o \
	  tpmux.o doelz.o \
	  $(SOBJS)

.PHONY : clean dep kit

ifeq ("$(HOST)","Darwin")

# Mac

.PHONY : dtcyber 

dtcyber:
	mkdir -p g3; \
	cd g3; \
	ln -sf ../Makefile.* .; \
	$(MAKE) -f ../Makefile gxdtcyber EXTRACFLAGS="$(G3CFLAGS) $(EXTRACFLAGS)" VPATH=.. DUAL=$(DUAL) PTERMVERSION=xxx
	mkdir -p g5; \
	cd g5; \
	ln -sf ../Makefile.* .; \
	$(MAKE) -f ../Makefile gxdtcyber EXTRACFLAGS="$(G5CFLAGS) $(EXTRACFLAGS)" VPATH=.. DUAL=$(DUAL) DUAL_HOST_CPUS=1 PTERMVERSION=xxx
	lipo -create -output dtcyber g3/gxdtcyber g5/gxdtcyber

gxdtcyber: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $+ $(LIBS) $(PTHLIBS)

clean:
	rm -rf *.o *.d *.pcf g3 g5 dd60 dtoper pterm pterm*.dmg Pterm.app

blackbox: blackbox.o g3/niu.o g3/charset.o g3/dtnetsubs.o
	$(CC) $(LDFLAGS) -o $@ $+ $(LIBS) $(THRLIBS)

else

# not Mac

dtcyber: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $+ $(LIBS) $(THRLIBS)

blackbox: blackbox.o niu.o charset.o dtnetsubs.o
	$(CC) $(LDFLAGS) -o $@ $+ $(LIBS) $(THRLIBS)

clean:
	rm -f *.d *.o *.pcf dtcyber dd60 dtoper pterm pterm*.zip pterm*.tar.gz
endif

kit:	pterm-kit

buildall: clean all

dep:	pterm.dep dd60.dep dtoper.dep $(OBJS:.o=.d) blackbox.d

ifneq ($(MAKECMDGOALS),clean)
include  $(OBJS:.o=.d) blackbox.d
endif

#---------------------------  End Of File  --------------------------------
