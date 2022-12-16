.PHONY: clean test


all: BMinorParser_X86_64 BMinorParser_llvm

FLAGS = -Wall -g
FLAGS += -I./

BMinorParser_X86_64: tokenize.l grammars.y ast.c symtable.c asmgen_x86_64.c  main.c
	bison -d --report=all grammars.y
	flex -o tokenize.lex.c tokenize.l
	gcc -o $@ ${FLAGS} grammars.tab.c tokenize.lex.c ast.c symtable.c asmgen_x86_64.c main.c -lfl -lut

BMinorParser_llvm: tokenize.l grammars.y ast.c symtable.c asmgen_llvm.c  main.c
	bison -d --report=all grammars.y
	flex -o tokenize.lex.c tokenize.l
	gcc -o $@ ${FLAGS} -DCODEGEN_LLVM grammars.tab.c tokenize.lex.c ast.c symtable.c asmgen_llvm.c main.c -lfl -lut

test: BMinorParser_X86_64 BMinorParser_llvm test*.txt
	./BMinorParser_X86_64 test_function.txt 1> test_function.s
	./BMinorParser_X86_64 test_gcd.txt 1> test_gcd.s
	gcc test_function.s -o test_function
	- ./test_function 
	gcc test_gcd.s -o test_gcd
	- ./test_gcd
	
	./BMinorParser_llvm test_function.txt 1> test_function.ll
	./BMinorParser_llvm test_gcd.txt 1> test_gcd.ll
	llvm-as test_function.ll -o test_function.bc
	- lli ./test_function.bc
	llvm-as test_gcd.ll -o test_gcd.bc
	- lli ./test_gcd.bc

clean:
	rm -rf *.o
	rm -rf BMinorParser
	rm -rf test_function
	rm -rf test_gcd
