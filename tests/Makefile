# header location
IDIR=../include/

# compile info
CC=clang
CFLAGS=-Wall -I$(IDIR)
LIBS=-lcrypto -lssl

# prepend IDIR to header depency names
_DEPS = rsa.h pad.h crypto.h
DEPS = $(patsubst %,$(IDIR)%,$(_DEPS))

all: rsa pad key_tests pad_tests

# compile objects
rsa: ../src/rsa.c $(DEPS)
		$(CC) -c $^ $(CFLAGS)

pad: ../src/pad.c $(DEPS)
		$(CC) -c $^ $(CFLAGS)

# link libs
key_tests: key_tests.c ../src/rsa.o
		$(CC) -o $@ $^ $(LIBS) $(CFLAGS)

pad_tests: pad_tests.c ../src/pad.o
		$(CC) -o $@ $^ $(LIBS) $(CFLAGS)

clean:
		rm -f *~ *.o $(IDIR)*.gch ../src/*.o
		rm key_tests pad_tests
