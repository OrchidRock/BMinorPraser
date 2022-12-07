%{
#include<stdio.h>

extern int yyparse();
%}

/* Keyword */
%token TOKEN_IF TOKEN_FOR TOKEN_RETURN TOKEN_PRINT 
%token TOKEN_BOOL TOKEN_INTEGER TOKEN_CHAR TOKEN_TYPE_STRING TOKEN_ARRAY TOKEN_VOID
%token TOKEN_FUNC 

/* Delimiter */
%token TOKEN_COLON TOKEN_SEMI TOKEN_LPAREN TOKEN_RPAREN
%token TOKEN_LBRACE TOKEN_RBRACE
%token TOKEN_LSQB TOKEN_RSQB
%token TOKEN_COMMA

/* Operator */
%token TOKEN_ASSIGN TOKEN_OR TOKEN_AND
%token TOKEN_LESS TOKEN_LESSEQU TOKEN_GREATER TOKEN_GREATEREQU TOKEN_EQUAL TOKEN_NOTEQUAL
%token TOKEN_ADD TOKEN_MINUS TOKEN_MUL TOKEN_DIV TOKEN_MOD TOKEN_EXPON
%token TOKEN_BANG
%token TOKEN_INC TOKEN_DEC

/* literal*/
%token TOKEN_NUMBER TOKEN_STRING TOKEN_NAME

%%

program: decl_list 
        { printf("program accept.\n"); }
    ;
    
decl_list:decl
    | decl_list decl
        {}
    ;

decl: TOKEN_NAME TOKEN_COLON type TOKEN_SEMI 
        {printf("Done: decl_not_assign %d\n", $3); }
    | TOKEN_NAME TOKEN_COLON type TOKEN_ASSIGN expr TOKEN_SEMI
        {printf("Done: decl_assign %d\n", $5);}
    | TOKEN_NAME TOKEN_COLON type TOKEN_ASSIGN stmt_suite
        {printf("Done: decl_func\n"); }
    ;


type: TOKEN_BOOL
    | TOKEN_INTEGER
    | TOKEN_CHAR
    | TOKEN_TYPE_STRING
    | TOKEN_ARRAY
    | TOKEN_VOID
    | TOKEN_FUNC type TOKEN_LPAREN funcarg TOKEN_RPAREN
    ;


funcarg: {} 
    | TOKEN_NAME TOKEN_COLON type 
    | TOKEN_NAME TOKEN_COLON type TOKEN_COMMA funcarg
    ;


simple_stmt: small_stmt
    | simple_stmt small_stmt
    ;    

stmt_suite: TOKEN_LBRACE simple_stmt TOKEN_RBRACE
    ;

small_stmt: TOKEN_RETURN expr TOKEN_SEMI 
    | TOKEN_PRINT expr TOKEN_SEMI   {printf("print stmt\n");}
    | TOKEN_IF TOKEN_LPAREN expr TOKEN_RPAREN stmt_suite
        {printf("Get IF.\n");}
    | TOKEN_FOR TOKEN_LPAREN expr TOKEN_SEMI expr TOKEN_SEMI expr TOKEN_RPAREN stmt_suite
        {printf("Get For.\n");}
    | decl
    | expr TOKEN_SEMI
    ;

//expr_list:
//    | expr_list expr
//    ;

expr: expr_or {printf("get expr\n"); }
    |TOKEN_NAME TOKEN_ASSIGN expr_or
    ;

expr_or: expr_and
    | expr_or TOKEN_OR expr_and
    ;

expr_and: expr_comp
    | expr_and TOKEN_AND expr_comp
    ;

expr_comp: expr_arith
    | expr_comp comp_op expr_arith
    ;

comp_op: TOKEN_LESS
    | TOKEN_LESSEQU
    | TOKEN_GREATER
    | TOKEN_GREATEREQU
    | TOKEN_EQUAL
    | TOKEN_NOTEQUAL
    ;

expr_arith: expr_factor
    | expr_arith TOKEN_ADD expr_factor
    | expr_arith TOKEN_MINUS expr_factor
    ;

expr_factor: expr_expon
    | expr_factor TOKEN_MUL expr_expon
    | expr_factor TOKEN_DIV expr_expon
    | expr_factor TOKEN_MOD expr_expon
    ;

expr_expon: expr_unary
    | expr_expon TOKEN_EXPON expr_unary
    ;

expr_unary: expr_incdec
    | TOKEN_BANG expr_incdec
    | TOKEN_MINUS expr_incdec
    ;

expr_incdec: expr_trailer
    | TOKEN_INC expr_trailer
    | TOKEN_DEC expr_trailer
    ;

expr_trailer: expr_atom
    | TOKEN_LPAREN expr TOKEN_RPAREN
    | TOKEN_NAME TOKEN_LSQB TOKEN_NUMBER TOKEN_RSQB
    | TOKEN_NAME TOKEN_LPAREN expr_callarg TOKEN_RPAREN
    ;

expr_callarg: expr
        | expr TOKEN_COMMA expr_callarg

expr_atom: TOKEN_NAME {printf("get name\n");}
    | TOKEN_NUMBER {printf("get number: %d\n", yylval);}
    | TOKEN_STRING
    ; 

%%



void main(){
    if(yyparse() == 0){
        printf("Parse successful!\n");
    } else {
        printf("Parse failed.\n");
    }
}

void yyerror(char* s){
    printf("error: %s\n", s);
}
