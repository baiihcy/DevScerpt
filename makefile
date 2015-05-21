include ../../../mhead

CFLAGS= -g -Wall -fPIC -I/usr/local/arm/include
LDFLAGS= -shared -L/usr/local/arm/lib

DEVNAME= luadev.so
all: $(DEVNAME) 
	
SRC= trlist.c sendlist.c dev.c main.c DataProcess.c luascript.c scriptif.c
TGT=$(SRC:.c=.o) ../../../txj/libio.o

$(SRC):types.h trlist.h sendlist.h dev.h main.h DataProcess.h luascript.h scriptif.h
	@touch $@

clean:
	-rm -f  *.gdb *.elf *.o *.so
	
%.o: %.c 
	$(CC) -c $(CFLAGS) $?

$(DEVNAME): $(TGT)
	$(CC) $(LDFLAGS) -o $@ $(TGT) -dl -llua -ldl -lm
	cp ./$(DEVNAME) ../../../bin/devices
