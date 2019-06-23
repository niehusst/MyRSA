# header location
IDIR=../include/

# compile info
CC=clang
CFLAGS=-Wall -I$(IDIR)
LIBS=-lm -lcrypto -lssl

# prepend IDIR to header depency names
_DEPS = rsa.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

# all object files that need linking together to run a demo
OBJ=rsa.o

all: rsa test

# compile rsa object
rsa: rsa.c $(DEPS)
		$(CC) -c $^ $(CFLAGS)

# TODO compile echo server object

# link object files together
demo: $(OBJ)
		$(CC) $(LIBS) -o $@ $^ $(CFLAGS)

clean:
		rm -f *~ *.o $(IDIR)*~ $(IDIR)*.gch
		rm -f demo
