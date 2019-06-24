# header location
IDIR=../include/

# compile info
CC=clang
CFLAGS=-Wall -I$(IDIR)
LIBS=-lcrypto -lssl

# prepend IDIR to header depency names
_DEPS = rsa.h pad.h crypto.h
DEPS = $(patsubst %,$(IDIR)%,$(_DEPS))

# all object files that need linking together to run a demo
OBJ=rsa.o pad.o crypto.o

all: rsa pad server client

# compile rsa object
rsa: rsa.c $(DEPS)
		$(CC) -c $^ $(CFLAGS)

# compile text padding object
pad: pad.c $(DEPS)
		$(CC) -c $^ $(CFLAGS)

crypto: crypto.c $(DEPS)
		$(CC) -c $^ $(CFLAGS)

# link and compile echo server executable
server: server.c $(OBJ)
		$(CC) $(LIBS) -o $@ $^ $(CFLAGS)

# link and compile echo client executable
client: client.c $(OBJ)
		$(CC) $(LIBS) -o $@ $^ $(CFLAGS)

clean:
		rm -f *~ *.o $(IDIR)*~ $(IDIR)*.gch
		rm -f client server
