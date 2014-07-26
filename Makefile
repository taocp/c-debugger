CFLAGS=-g -Wall

all:xibugger traced

xibugger:xibugger.c 

traced:traced.c

run:
	./xibugger traced

clean:
	rm -f ./xibugger ./traced
