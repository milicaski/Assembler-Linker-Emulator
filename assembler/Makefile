all: assembler

clean:
	rm -f assembler lex.yy.c parser.tab.c parser.tab.h

parser.tab.c: src/parser.y
	bison -d src/parser.y

lex.yy.c: parser.tab.c
	flex src/scanner.l

assembler: lex.yy.c
	g++ -o assembler src/assembler.cpp src/Impl.cpp parser.tab.c lex.yy.c 

test: assembler
	./assembler -o ../interrupts.o tests/interrupts.s
	./assembler -o ../main.o tests/main.s