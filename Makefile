#--------------------------------------------------------------------------
#
#   Copyright (c) 2004, Tom Hunter (see license.txt)
#
#   Name: Makefile.x11
#
#   Description:
#       Build X11 version of Desktop Cyber emulation.
#
#--------------------------------------------------------------------------

#LIBS   = -lm -lX11 -lpthreads
LIBS    = -lm -lX11 -lpthread
#LIBS   = -lm -lX11
LDFLAGS = -s -L/usr/X11R6/lib
INCL    = -I/usr/X11R6/include

#CFLAGS = -O2 -I. $(INCL) -Wall -Wshadow -Wpointer-arith -Wmissing-prototypes 
CFLAGS  = -O2 -I. $(INCL)

OBJS    =   channel.o               \
            charset.o               \
            console.o               \
            cp3446.o                \
            cpu.o                   \
            cr3447.o                \
            cr405.o                 \
            dcc6681.o               \
            dd6603.o                \
            dd8xx.o                 \
            ddp.o                   \
            deadstart.o             \
            device.o                \
            dump.o                  \
            float.o                 \
            init.o                  \
            log.o                   \
            lp1612.o                \
            lp3000.o                \
            main.o                  \
            mt607.o                 \
            mt669.o                 \
            mux6676.o               \
            niu.o                   \
            operator.o              \
            pp.o                    \
            rtc.o                   \
            shift.o                 \
            tpmux.o                 \
            trace.o                 \
            window_x11.o            
 

dtcyber: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

all: clean dtcyber

clean:
	rm -f *.o

.c.o:
	$(CC) $(CFLAGS) -c $*.c

#---------------------------  End Of File  --------------------------------
