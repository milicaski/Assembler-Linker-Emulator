all: linker

clean:
	rm -f linker

linker:
	g++ -o linker src/linker.cpp src/ImplLinker.cpp

test: linker
	./linker -hex -place=ivt@0x0000 -o ../program.hex ../interrupts.o ../main.o