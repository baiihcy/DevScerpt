include ../../../mhead
#CC=
#SYSOBJECTS=
#SYSCFLAGS=
#SYSLDFLAGS=
#TO_PREFIX=

DEVNAME= luadev.so
MYCFLAGS= -g  -fPIC -I/usr/local/include
MYLDFLAGS= -shared -rdynamic -L/usr/local/lib

ifeq ($(CC),arm-linux-gcc)
	MYCFLAGS= -g -Wall -fPIC -I/usr/local/arm/include
	MYLDFLAGS= -shared -L/usr/local/arm/lib
endif

ifeq ($(CC),powerpc-linux-gcc)
	MYCFLAGS= -g -Wall -fPIC -I/usr/local/powerpc/include
	MYLDFLAGS= -shared -L/usr/local/powerpc/lib
endif

SRC= trlist.c sendlist.c dev.c main.c DataProcess.c luascript.c scriptif.c
TGT=$(SRC:.c=.o)



$(ALL): $(DEVNAME) 

$(SRC):types.h trlist.h sendlist.h dev.h main.h DataProcess.h luascript.h scriptif.h
	@touch $@
	
%.o: %.c 
	$(CC) -c $(SYSCFLAGS) $(MYCFLAGS) $?

$(DEVNAME): $(TGT)
	$(CC) $(SYSLDFLAGS) $(MYLDFLAGS) -o $@ $(SYSOBJECTS) $(TGT) -dl -llua -ldl -lm
	cp ./$(DEVNAME) $(TO_PREFIX)/devices

clean:
	rm -f  *.gdb *.elf *.o *.so
	
