.PHONY: clean test


all: BMinorParser

FLAGS = -I./

BMinorParser: tokenize.l grammars.y ast.c symtable.c  main.c
	bison -d --report=all grammars.y
	flex -o tokenize.lex.c tokenize.l
	gcc -o $@ ${FLAGS} grammars.tab.c tokenize.lex.c ast.c symtable.c main.c -lfl -lut


test: BMinorParser test*.txt
	./BMinorParser test_typechecking.txt

clean:
	rm -rf *.o
	rm -rf BMinorParser
