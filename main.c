#include<stdio.h>
#include<stdlib.h>

#include "ast.h"

extern int yyparse();
extern int lineno;
extern struct decl* parser_result;

extern FILE* yyin;

int main(int argc, char* argv[]){
    FILE* fp = NULL;
    if(argc > 1){
        fp = fopen(argv[1], "r");
        yyin = fp;
    }
    if(yyparse() == 0){
        printf("Parse successful!\n");
    } else {
        printf("Parse failed.\n");
    }

    if(fp != NULL){
        fclose(fp);
    }
    
    /* Create SymbolTable and TypeChecking.*/ 
    decl_resolve(parser_result);

    scope_exit();

    decl_destory(parser_result);
    
    // printf("\n Finished. Done. \n");

    return 0;
}

void yyerror(char* s){
    fprintf(stderr, "error: %s, at line: %d\n", s, lineno+1);
}
