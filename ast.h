#ifndef __AST_H__
#define __AST_H__

#include "symtable.h"

typedef enum{
    TYPE_VOID,
    TYPE_BOOLEAN,
    TYPE_CHARACTER,
    TYPE_INTEGER,
    TYPE_STRING,
    TYPE_ARRAY,
    TYPE_FUNCTION
} type_t;

struct type {
    type_t kind;
    struct type *subtype;
    struct param_list *params;
};

struct param_list{
    char* name;
    struct type* type;
    struct param_list *next;
    struct symbol* symbol;
};

struct decl {
    char* name;
    struct type * type;
    struct expr * value;
    struct stmt * code;
    struct decl * next;
    struct symbol* symbol; /*  */
};


typedef enum {
    STMT_DECL,
    STMT_EXPR,
    STMT_IF_ELSE,
    STMT_FOR,
    STMT_PRINT,
    STMT_RETURN,
    STMT_BLOCK
} stmt_t;

struct stmt {
    stmt_t kind;
    struct decl *decl;
    struct expr *init_expr;
    struct expr *expr;
    struct expr *next_expr;
    struct stmt *body;
    struct stmt *else_body;
    struct stmt *next;
};

typedef enum{
    EXPR_ADD, EXPR_MINUS, EXPR_MUL, EXPR_DIV, EXPR_MOD,
    EXPR_EXPON, EXPR_OR, EXPR_AND, EXPR_ASSIGN, 
    EXPR_LESS, EXPR_LESSEQU, EXPR_GREATER, EXPR_GREATEREQU, EXPR_EQUAL, 
    EXPR_NOTEQUAL,
    EXPR_BANG,
    EXPR_LOGNOT,
    EXPR_INC, EXPR_DEC,
    EXPR_SUBCRIPT, EXPR_CALL, EXPR_ARG,

    EXPR_NAME, EXPR_NUMBER, EXPR_STRING, EXPR_CHAR,
}expr_t;

struct expr {
    expr_t kind;
    struct expr *left;
    struct expr *right;
    const char *name;
    int integer_value;
    const char * string_literal;
    struct symbol* symbol;
};



struct decl* decl_create(char*, struct type*,struct expr *,
                struct stmt*, struct decl*);
void decl_destory(struct decl* root);

struct stmt* stmt_create(stmt_t kind, struct decl *decl, 
                struct expr *init_expr, struct expr *expr, 
                struct expr *next_expr, struct stmt *body,
                struct stmt *else_body, struct stmt *next);

struct expr* expr_create(expr_t kind, struct expr* left, struct expr* right);
struct expr* expr_create_name(char* name);
struct expr* expr_create_string(char* str);
struct expr* expr_create_number(int number);
struct expr* expr_create_char(int ch);


struct type* type_create(type_t, struct type*, struct param_list*);
struct param_list* param_list_create(char*, struct type*, struct param_list*);

/* Name Reloluting. */
void decl_resolve(struct decl *d);
void expr_resolve(struct expr *e);
void stmt_resolve(struct stmt* s);
void param_list_resolve(struct param_list* pl);

char* strcopy(const char* name);
#endif // __AST_H__
