.PHONY: clean


all: BMinorParser

FLAGS = -I./

BMinorParser: tokenize.l grammars.y ast.c
	bison -d --report=all grammars.y
	flex -o tokenize.lex.c tokenize.l
	gcc -o $@ ${FLAGS} grammars.tab.c tokenize.lex.c ast.c -lfl 




clean:
	rm -rf *.o
	rm -rf BMinorParser
