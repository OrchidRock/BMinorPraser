.PHONY: clean


all: BMinorParser


BMinorParser: tokenize.l grammars.y
	bison -d --report=all grammars.y
	flex -o tokenize.lex.c tokenize.l
	gcc -o $@ grammars.tab.c tokenize.lex.c -lfl 

clean:
	rm -rf *.o
	rm -rf BMinorParser
