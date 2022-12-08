%{
#include "ast.h"
struct decl* parser_result = NULL;
%}

/* Keyword */
%token TOKEN_IF TOKEN_ELSE TOKEN_FOR TOKEN_RETURN TOKEN_PRINT 
%token TOKEN_BOOL TOKEN_INTEGER TOKEN_TYPE_CHAR TOKEN_TYPE_STRING TOKEN_ARRAY TOKEN_VOID
%token TOKEN_FUNC TOKEN_BOOL_TRUE TOKEN_BOOL_FALSE 

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
%token TOKEN_NUMBER TOKEN_STRING TOKEN_NAME TOKEN_CHAR


%union {
    struct decl* decl;
    struct stmt* stmt;
    struct type* type;
    struct expr* expr;
    struct param_list* param_list; 
    char* name;
};

%type <decl> program decl decl_list
%type <type> type
%type <stmt> stmt_suite simple_stmt small_stmt
%type <name> name
%type <param_list> funcarg
%type <expr> expr expr_or expr_and expr_atom expr_comp expr_incdec
%type <expr> expr_arith expr_expon expr_unary expr_trailer expr_callarg expr_factor
%type <expr> expr_name expr_number
%%

program: decl_list 
        { parser_result = $1;}
    ;
    
decl_list:decl  {$$ = $1; }
    | decl decl_list
        {$$ = $1; $1->next = $2;}
    ;

decl: name TOKEN_COLON type TOKEN_SEMI 
        {$$ = decl_create($1, $3, NULL, NULL, NULL); }
    | name TOKEN_COLON type TOKEN_ASSIGN expr TOKEN_SEMI
        {$$ = decl_create($1, $3, $5, NULL, NULL);}
    | name TOKEN_COLON type TOKEN_ASSIGN stmt_suite
        {$$ = decl_create($1, $3, NULL, $5, NULL); }
    ;

name: TOKEN_NAME { $$ = yylval.name; yylval.name = NULL;}


type: TOKEN_BOOL
        {$$ = type_create(TYPE_BOOLEAN, NULL, NULL);}
    | TOKEN_INTEGER
        {$$ = type_create(TYPE_INTEGER, NULL, NULL);}
    | TOKEN_TYPE_CHAR
        {$$ = type_create(TYPE_CHARACTER, NULL, NULL);}
    | TOKEN_TYPE_STRING
        {$$ = type_create(TYPE_STRING, NULL, NULL);}
    | TOKEN_ARRAY TOKEN_LSQB TOKEN_RSQB type
        {$$ = type_create(TYPE_ARRAY, $4, NULL); }
    | TOKEN_VOID
        {$$ = type_create(TYPE_VOID, NULL, NULL); }
    | TOKEN_FUNC type TOKEN_LPAREN funcarg TOKEN_RPAREN
        {$$ = type_create(TYPE_FUNCTION, $2, $4); }
    ;


funcarg: {$$ = NULL;} 
    | name TOKEN_COLON type
        {$$ = param_list_create($1, $3, NULL);} 
    | name TOKEN_COLON type TOKEN_COMMA funcarg
        {$$ = param_list_create($1, $3, $5);}
    ;


stmt_suite: TOKEN_LBRACE simple_stmt TOKEN_RBRACE
            {$$ = stmt_create(STMT_BLOCK, NULL, NULL, NULL, NULL, $2, NULL, NULL);}
    ;

simple_stmt: small_stmt {$$ = $1;}
    | small_stmt simple_stmt
        {$$ = $1; $1->next = $2;}
    ;    

small_stmt: TOKEN_RETURN expr TOKEN_SEMI
            {$$ = stmt_create(STMT_RETURN, NULL, NULL, $2, NULL, NULL, NULL, NULL);} 
    | TOKEN_PRINT expr TOKEN_SEMI
        {$$ = stmt_create(STMT_PRINT, NULL, NULL, $2, NULL, NULL, NULL, NULL);}     
    | TOKEN_IF TOKEN_LPAREN expr TOKEN_RPAREN stmt_suite
        {$$ = stmt_create(STMT_IF_ELSE, NULL, NULL, $3, NULL, $5, NULL, NULL);}     
    | TOKEN_IF TOKEN_LPAREN expr TOKEN_RPAREN stmt_suite TOKEN_ELSE stmt_suite 
        {$$ = stmt_create(STMT_IF_ELSE, NULL, NULL, $3, NULL, $5, $7, NULL);}     
    | TOKEN_FOR TOKEN_LPAREN expr TOKEN_SEMI expr TOKEN_SEMI expr TOKEN_RPAREN stmt_suite
        {$$ = stmt_create(STMT_FOR, NULL, $3, $5, $7, $9, NULL, NULL);}     
    | decl
        {$$ = stmt_create(STMT_DECL, $1, NULL, NULL, NULL, NULL, NULL, NULL);}     
    | expr TOKEN_SEMI
        {$$ = stmt_create(STMT_EXPR, NULL, NULL, $1, NULL, NULL, NULL, NULL);}
    ;

//expr_list:
//    | expr_list expr
//    ;

expr: expr_or {$$ = $1;}
    //| expr_name TOKEN_ASSIGN expr_or
    | expr_trailer TOKEN_ASSIGN expr_or
        {$$ = expr_create(EXPR_ASSIGN, $1, $3);}
    ;

expr_or: expr_and {$$ = $1;}
    | expr_or TOKEN_OR expr_and
        {$$ = expr_create(EXPR_OR, $1, $3);}
    ;

expr_and: expr_comp {$$ = $1;}
    | expr_and TOKEN_AND expr_comp
        {$$ = expr_create(EXPR_AND, $1, $3);}
    ;

expr_comp: expr_arith {$$ = $1;}
    | expr_comp TOKEN_LESS expr_arith
        {$$ = expr_create(EXPR_LESS, $1, $3);}
    | expr_comp TOKEN_LESSEQU expr_arith
        {$$ = expr_create(EXPR_LESSEQU, $1, $3);}
    | expr_comp TOKEN_GREATEREQU expr_arith
        {$$ = expr_create(EXPR_GREATEREQU, $1, $3);}
    | expr_comp TOKEN_GREATER expr_arith
        {$$ = expr_create(EXPR_GREATER, $1, $3);}
    | expr_comp TOKEN_EQUAL expr_arith
        {$$ = expr_create(EXPR_EQUAL, $1, $3);}
    | expr_comp TOKEN_NOTEQUAL expr_arith
        {$$ = expr_create(EXPR_NOTEQUAL, $1, $3);}
    ;

expr_arith: expr_factor {$$ = $1;}
    | expr_arith TOKEN_ADD expr_factor
        {$$ = expr_create(EXPR_ADD, $1, $3);}
    | expr_arith TOKEN_MINUS expr_factor
        {$$ = expr_create(EXPR_MINUS, $1, $3);}
    ;

expr_factor: expr_expon {$$ = $1;}
    | expr_factor TOKEN_MUL expr_expon
        {$$ = expr_create(EXPR_MUL, $1, $3);}
    | expr_factor TOKEN_DIV expr_expon
        {$$ = expr_create(EXPR_DIV, $1, $3);}
    | expr_factor TOKEN_MOD expr_expon
        {$$ = expr_create(EXPR_MOD, $1, $3);}
    ;

expr_expon: expr_unary {$$ = $1;}
    | expr_expon TOKEN_EXPON expr_unary
        {$$ = expr_create(EXPR_EXPON, $1, $3);}
    ;

expr_unary: expr_incdec {$$ = $1;}
    | TOKEN_BANG expr_incdec
        {$$ = expr_create(EXPR_BANG, $2, NULL);}
    | TOKEN_MINUS expr_incdec
        {$$ = expr_create(EXPR_LOGNOT, $2, NULL);}
    ;

expr_incdec: expr_trailer {$$ = $1;}
    | TOKEN_INC expr_trailer
        {$$ = expr_create(EXPR_INC, $2, NULL);}
    | TOKEN_DEC expr_trailer
        {$$ = expr_create(EXPR_DEC, $2, NULL);}
    ;

expr_trailer: expr_atom {$$ = $1;}
    | TOKEN_LPAREN expr TOKEN_RPAREN
        {$$ = $2;}
    | expr_name TOKEN_LSQB expr_number TOKEN_RSQB
        {$$ = expr_create(EXPR_SUBCRIPT, $1, $3);}
    | expr_name TOKEN_LPAREN expr_callarg TOKEN_RPAREN
        {$$ = expr_create(EXPR_CALL, $1, $3);}
    ;

expr_callarg: {$$ = NULL;} 
        | expr {$$ = expr_create(EXPR_ARG, $1, NULL);}
        | expr TOKEN_COMMA expr_callarg
            {$$ = expr_create(EXPR_ARG, $1, $3); }

expr_atom: expr_name {$$ = $1;}
    | expr_number {$$ = $1;}
    | TOKEN_STRING 
        {$$ = expr_create_string(yylval.name);}
    | TOKEN_CHAR
        {$$ = expr_create_number((int)yylval.name);}
    | TOKEN_BOOL_TRUE
        {$$ = expr_create_number(1);}
    | TOKEN_BOOL_FALSE
        {$$ = expr_create_number(0);}
    ;

expr_name: TOKEN_NAME
            {$$ = expr_create_name(yylval.name); yylval.name = NULL;}
        ;

expr_number: TOKEN_NUMBER
            {$$ = expr_create_number((int)yylval.name);}
    ;

%%

