# compile info
CC=clang
CFLAGS=-Wall
LIBS=-lm -lcrypto -lssl

# header location
#IDIR=../include/


all: rsa

rsa: rsa.c
		$(CC) $(LIBS) -o $@ $^ $(CFLAGS)

clean:
		rm -f *~ *.o rsa
