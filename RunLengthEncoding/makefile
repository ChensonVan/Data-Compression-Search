CC=g++
all: rlencode rldecode

rlencode: rlencode.o
	$(CC) rlencode.o -o rlencode

rldecode: rldecode.o
	$(CC) rldecode.o -o rldecode

clean:
	rm *.o rldecode rlencode
