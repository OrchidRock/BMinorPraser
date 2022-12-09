.PHONY: clean test


all: BMinorParser

FLAGS = -Wall -g
FLAGS += -I./

BMinorParser: tokenize.l grammars.y ast.c symtable.c asmgen_x86_64.c  main.c
	bison -d --report=all grammars.y
	flex -o tokenize.lex.c tokenize.l
	gcc -o $@ ${FLAGS} grammars.tab.c tokenize.lex.c ast.c symtable.c asmgen_x86_64.c main.c -lfl -lut


test: BMinorParser test*.txt
	./BMinorParser test_function.txt 1> test_function.s
	gcc test_function.s -o test_function
	./test_function

clean:
	rm -rf *.o
	rm -rf BMinorParser
