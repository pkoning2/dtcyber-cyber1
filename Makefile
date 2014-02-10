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

ifeq ($(MAKECMDGOALS),)
MAKECMDGOALS=all
endif

DEPFILES=

include Makefile.common

CDEBUG = -DCcDebug=1

OBJS    = main.o init.o trace.o dump.o \
          device.o channel.o cpu.o pp.o float.o shift.o operator.o \
          deadstart.o console.o cr405.o dd6603.o dd8xx.o mux6676.o \
          lp1612.o mt607.o mt669.o dcc6681.o rtc.o log.o \
	  cr3447.o ddp.o niu.o lp3000.o cp3446.o \
	  tpmux.o dtdisksubs.o ext.o pni.o \
	  $(PWD)/charset.o $(PWD)/dtnetsubs.o

CC      = gcc
CXX     = g++

ifneq ("$(NPU_SUPPORT)","")
OBJS +=	  npu_async.o npu_bip.o npu_hip.o npu_svm.o npu_tip.o npu_net.o
VERSIONCFLAGS +=   -DNPU_SUPPORT=1
endif

.PHONY : clean dep kit all

ifeq ("$(HOST)","Darwin")

# Mac

ifeq ("$(SDKDIR)","")
OSXVER ?= 10.6
OSXMIN ?= 10.4
SDKDIR := /Developer/SDKs/MacOSX$(OSXVER).sdk
endif
CLANG := $(shell gcc --version 2>/dev/null| fgrep LLVM)
ifneq ("$(CLANG)", "")
CXXLIB := -stdlib=libstdc++
endif
LIBS    +=  -Wl,-syslibroot,$(SDKDIR) -L$(SDKDIR)/usr/lib -L/usr/lib
INCL    += -isysroot $(SDKDIR)
LDFLAGS +=  -mmacosx-version-min=$(OSXMIN) $(CXXLIB)
CFLAGS  +=  -mmacosx-version-min=$(OSXMIN) $(CXXLIB)
X86ARCHFLAGS = -arch i386
X86_64ARCHFLAGS = -arch x86_64
TOOLLDFLAGS = -arch i386

MACTARGETS=x86 x86_64

.PHONY : dtcyber 

all: dtcyber Pterm.app dtoper.app dd60.app blackbox

dtcyber:
	mkdir -p $(MACTARGETS)
	( cd x86 && \
	ln -sf ../Makefile.* . && \
	$(MAKE) -f ../Makefile gxdtcyber EXTRACFLAGS="$(X86CFLAGS) $(EXTRACFLAGS) -DARCHNAME='\"i386\"'" VPATH=.. DUAL=$(DUAL) CC=$(CC) PTERMVERSION=xxx ARCHCFLAGS="$(X86ARCHFLAGS)" LDFLAGS="$(LDFLAGS)" )
	( cd x86_64 && \
	ln -sf ../Makefile.* . && \
	$(MAKE) -f ../Makefile gxdtcyber EXTRACFLAGS="$(X86_64CFLAGS) $(EXTRACFLAGS) -DARCHNAME='\"x86_64\"'" VPATH=.. DUAL=$(DUAL) CC=$(CC) PTERMVERSION=xxx ARCHCFLAGS="$(X86_64ARCHFLAGS)" LDFLAGS="$(LDFLAGS)" )
	lipo -create -output dtcyber `for d in $(MACTARGETS); do echo $$d/gxdtcyber; done`

gxdtcyber: $(OBJS)
	$(CC) $(LDFLAGS) $(ARCHCFLAGS) -o $@ $+ $(LIBS) $(PTHLIBS)

clean:
	rm -rf *.o *.d *.i *.ii *.pcf x86 x86_64 dd60 dtoper pterm pterm*.dmg Pterm.app dtoper.app dd60.app

blackbox: blackbox.o $(SOBJS)
	$(CC) $(LDFLAGS) $(TOOLLDFLAGS) -o $@ $+ $(LIBS) $(THRLIBS)

else

# not Mac

all: dtcyber pterm dtoper dd60 blackbox

dtcyber: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $+ $(LIBS) $(THRLIBS)

blackbox: blackbox.o $(SOBJS)
	$(CC) $(LDFLAGS) -o $@ $+ $(LIBS) $(THRLIBS)

clean:
	rm -f *.d *.o *.i *.ii *.pcf dtcyber dd60 dtoper pterm pterm*.zip pterm*.tar.gz
endif

kit:	pterm-kit

buildall: clean all

dep:	pterm.dep dd60.dep dtoper.dep $(OBJS:.o=.d) blackbox.d

# For dtcyber
DEPFILES+= $(OBJS:.o=.d) 
# For blackbox
DEPFILES+= blackbox.d niu.d charset.d dtnetsubs.d

include Makefile.wxpterm

# This must be last
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(DEPFILES),)
include $(DEPFILES)
endif
endif

#---------------------------  End Of File  --------------------------------
