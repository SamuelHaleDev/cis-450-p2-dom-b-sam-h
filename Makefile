CFLAGS= -g
LDFLAGS= -pthread
CC=g++

all: savingsProblemA savingsProblemB bridge boundedBuffer

# To make an executable
savingsProblemA: savingsProblemA.o 
	$(CC) $(LDFLAGS) -o savingsProblemA savingsProblemA.o

savingsProblemB: savingsProblemB.o 
	$(CC) $(LDFLAGS) -o savingsProblemB savingsProblemB.o

bridge: bridge.o 
	$(CC) $(LDFLAGS) -o bridge bridge.o

boundedBuffer: boundedBuffer.o 
	$(CC) $(LDFLAGS) -o boundedBuffer boundedBuffer.o


# To make an object from source
.c.o:
	$(CC) $(CFLAGS) -c $*.c

# clean out the dross
clean:
	-rm savingsProblemA savingsProblemB bridge boundedBuffer *.o

