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



#LIBS    = -lm -lX11 -lpthreads

LIBS    = -lm -lX11 -lpthread

#LIBS    = -lm -lX11

#LDFLAGS = -s -L/usr/X11R6/lib

LDFLAGS = -L/usr/X11R6/lib

INCL    = -I/usr/X11R6/include



#CFLAGS  = -O2 -I. $(INCL) -Wall -Wshadow -Wpointer-arith -Wmissing-prototypes 

CFLAGS  = -O2 -I. $(INCL)



OBJS    = main.o window_x11.o init.o trace.o charset.o dump.o \

          device.o channel.o cpu.o pp.o float.o shift.o operator.o \

          deadstart.o console.o cr405.o dd6603.o dd844.o mux6676.o \

          lp1612.o mt607.o mt669.o dcc6681.o rtc.o \

	  cr3447.o ddp.o niu.o lp3000.o



dtcyber: $(OBJS)

	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)



all: clean dtcyber



clean:

	rm -f *.o



.c.o:

	$(CC) $(CFLAGS) -c $*.c



#---------------------------  End Of File  --------------------------------

