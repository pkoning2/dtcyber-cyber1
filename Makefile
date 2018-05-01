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

.PHONY : clean kit all

ifeq ("$(HOST)","Darwin")

# Mac

LINK=$(CXX)

ifeq ("$(SDKDIR)","")
OSXMIN ?= 10.4
endif
CLANG := $(shell gcc --version 2>/dev/null| fgrep LLVM)
ifneq ("$(CLANG)", "")
AVX ?= -mavx2
ARCHCFLAGS ?= -arch i386 -arch x86_64 $(AVX)
ARCHLDFLAGS ?= -arch i386 -arch x86_64
OSXVER ?= 10.9
OSXMIN = 10.7
else
ARCHCFLAGS ?= -arch i386 -arch ppc -arch x86_64 -arch ppc64
ARCHLDFLAGS ?= -arch i386 -arch ppc -arch x86_64
OSXVER ?= 10.5
endif
SDKDIR := /Developer/SDKs/MacOSX$(OSXVER).sdk
LIBS    +=  -Wl,-syslibroot,$(SDKDIR) -L$(SDKDIR)/usr/lib 
INCL    += -isysroot $(SDKDIR) -I$(SDKDIR)/usr/include/c++/4.2.1 
ifneq ("$(OSXMIN)","")
OSXMINFLG = -mmacosx-version-min=$(OSXMIN)
endif
ARCHLDFLAGS +=  $(OSXMINFLG) $(CXXLIB)
ARCHCFLAGS  +=  $(OSXMINFLG) $(CXXLIB)
ARCHCCFLAGS ?= $(ARCHCFLAGS) $(AVX)

ifneq ("$(wildcard main.c)","")
all: dtcyber Pterm.app dtoper.app dd60.app blackbox
else
all: Pterm.app
endif

clean:
	rm -rf *.o *.d *.i *.ii *.pcf x86 x86_64 dd60 dtoper pterm pterm*.dmg Pterm.app *Pterm.pkg dtoper.app dd60.app pterm-*.tar.bz2

else

# not Mac

LINK=$(CXX)
ifneq ("$(wildcard main.c)","")
all: dtcyber pterm dtoper dd60 blackbox
else
all: pterm
endif

clean:
	rm -f *.d *.o *.i *.ii *.pcf dtcyber dd60 dtoper pterm pterm*.zip pterm*.tar.bz2
endif

dtcyber: $(OBJS)
	$(CC) $(LDFLAGS) $(LIBS) $(THRLIBS) $(ARCHCFLAGS) -o $@ $+

blackbox: blackbox.o $(SOBJS)
	$(CC) $(LDFLAGS) $(LIBS) $(THRLIBS) -o $@ $+

kit:	pterm-kit

buildall: clean all

ifneq ("$(wildcard main.c)","")
# For dtcyber
DEPFILES+= $(OBJS:.o=.d) 
# For blackbox
DEPFILES+= blackbox.d niu.d charset.d dtnetsubs.d
endif

ifneq ($(MAKECMDGOALS),dtcyber)
include Makefile.wxpterm
endif

# This must be last
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),info)
ifneq ($(MAKECMDGOALS),gxdtcyber)
ifneq ($(DEPFILES),)
include $(DEPFILES)
endif
endif
endif
endif

#---------------------------  End Of File  --------------------------------
