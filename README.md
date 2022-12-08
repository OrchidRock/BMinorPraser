BMinorParser
===========

This is a compiler-demo for the [B-Minor Program Language][https://www3.nd.edu/~dthain/courses/cse40243/fall2019/bminor.html#:~:text=This%20is%20an%20informal%20specification%20of%20B-Minor%202019%2C,the%20standard%20C%20library%2C%20within%20its%20defined%20types.].

### Dependence
`uthash` : The Hash Table structure implemented by C.


### Structure
```
.
├── ast.c           // The AST data structure.
├── ast.h           // The header file of AST.
├── grammars.y      // The bison script which represent grammars.
├── main.c          // entry point.
├── Makefile        
├── README.md   
├── symtable.c      // Symbol HashTable be orginized by stack.
├── symtable.h      // The header file of Symbol HashTable.
├── test.txt        // test file.
├── tokenize.l      // The flex script which represent token preser.

```

### Build

`$ make`


### test

`$ make test`

