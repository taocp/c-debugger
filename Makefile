CFLAGS=-g -Wall

all:xibugger traced

xibugger:xibugger.c ./lib/list.c ./dwarf/dwarf.c -lelf -ldwarf

traced:traced.c

run:
	./xibugger traced

clean:
	rm -f ./xibugger ./traced ./traced.func_addr
