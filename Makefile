CFLAGS=-g -Wall

all:debugger traced

debugger:debugger.c ./lib/list.c ./dwarf/dwarf.c -lelf -ldwarf

traced:traced.c

run:
	./debugger traced

clean:
	rm -f ./debugger ./traced ./traced.func_addr
