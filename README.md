BMinorParser
===========

This is a compiler-demo for the [B-Minor Program Language](https://www3.nd.edu/~dthain/courses/cse40243/fall2019/bminor.html).

### Dependence
`uthash` : The Hash Table structure implemented by C.

`flex`

`blson`


### Structure
```
.
├── ast.c           // The AST data structure.
├── ast.h           // The header file of AST.
├── asmgen_x86_64.c // The asmcode generator.
├── asmgen_x86_64.h
├── grammars.y      // The bison script which represent grammars.
├── main.c          // entry point.
├── Makefile        
├── README.md   
├── symtable.c      // Symbol HashTable be orginized by stack.
├── symtable.h      // The header file of Symbol HashTable.
├── test_function.txt
├── test_typechecking.txt
└── valgrind.output
├── tokenize.l      // The flex script which represent token preser.

```

### Build

`$ make`


### test

`$ make test`

