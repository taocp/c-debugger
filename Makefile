CFLAGS=-g -Wall

all:xibugger traced

xibugger:xibugger.c ./lib/list.c

traced:traced.c

run:
	./xibugger traced

clean:
	rm -f ./xibugger ./traced
