#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "ast.h"

static void type_destory(struct type*);
static void param_list_destory(struct param_list*);
static void expr_destory(struct  expr*);
static void stmt_destory(struct stmt*);

struct decl* decl_create(char* name,
                        struct type* type,
                        struct expr* value,
                        struct stmt* code,
                        struct decl* next){

    struct decl *d = malloc(sizeof(struct decl));
    d->name = name;
    d->type = type;
    d->value = value;
    d->code = code;
    d->next = next;
    
    printf("decl_create: %s done.\n", name); 

    return d;
}

void decl_destory(struct decl* root){
    if(root == NULL) return;

    if(root->name != NULL) free(root->name);
    
    type_destory(root->type);
    expr_destory(root->value);
    stmt_destory(root->code);
    
    if(root->next != NULL){
        decl_destory(root->next);
    }
    free(root);
    
    printf("%s: done.\n",__func__); 
}


struct type* type_create(type_t kind, struct type* stype, struct param_list* pl){
    struct type* t = malloc(sizeof(struct type));
    t->kind = kind;
    t->subtype = stype;
    t->params = pl;
    
    printf("%s: %d done.\n",__func__, kind); 

    return t;
}
static void type_destory(struct type* target_type){
    
    if(target_type == NULL) return;
    
    if(target_type->subtype != NULL){
        type_destory(target_type->subtype);
    }
    
    param_list_destory(target_type->params);
    
    free(target_type);
}

struct param_list* param_list_create(char* name, struct type* type,
                struct param_list* next_params){
    
    struct param_list* params = malloc(sizeof(struct param_list));
    params->name = name;
    params->type = type;
    params->next = next_params;
    return params;
}

static void param_list_destory(struct param_list* pl){
    if(pl == NULL) return;

    if(pl->name != NULL) free(pl->name);
    
    type_destory(pl->type);
    
    if(pl->next != NULL){
        param_list_destory(pl->next);
    }

    free(pl);
}



struct stmt* stmt_create(stmt_t kind, struct decl *decl, 
                struct expr *init_expr, struct expr *expr, 
                struct expr *next_expr, struct stmt *body,
                struct stmt *else_body, struct stmt *next){
    
    struct stmt* new_stmt = malloc(sizeof(struct stmt));
    new_stmt->kind = kind;
    new_stmt->decl = decl;
    new_stmt->init_expr = init_expr;
    new_stmt->expr = expr;
    new_stmt->next_expr = next_expr;
    new_stmt->body = body;
    new_stmt->else_body = else_body;
    new_stmt->next = next;

    return new_stmt;
    printf("%s: done. kind: %d \n",__func__, kind); 
    
}

static void stmt_destory(struct stmt* target_stmt){
    if(target_stmt == NULL) return;

    decl_destory(target_stmt->decl);
    expr_destory(target_stmt->init_expr);
    expr_destory(target_stmt->expr);
    expr_destory(target_stmt->next_expr);
    stmt_destory(target_stmt->body);
    stmt_destory(target_stmt->else_body);
    if(target_stmt->next){
        stmt_destory(target_stmt->next);
    }
    free(target_stmt);
    printf("%s: done.\n",__func__); 
}


struct expr* expr_create(expr_t kind, struct expr* left,
            struct expr* right){
    struct expr* new_expr = malloc(sizeof(struct expr));

    new_expr->kind = kind;
    new_expr->left = left;
    new_expr->right = right;
    new_expr->name = NULL;
    new_expr->string_literal = NULL;
    new_expr->integer_value = 0;

    printf("%s: done. kind: %d \n",__func__, kind); 
    
    return new_expr;
}

struct expr* expr_create_name(char* str){
    struct expr* new_expr = expr_create(EXPR_NAME, NULL, NULL);
    new_expr->name = str;
    
    printf("%s: done. name: %s \n",__func__, str); 
    
    return new_expr;
}
struct expr* expr_create_number(int number){
    struct expr* new_expr = expr_create(EXPR_NUMBER, NULL, NULL);
    new_expr->integer_value = number;
    
    printf("%s: done. number: %d \n",__func__, number); 
    
    return new_expr;
}

struct expr* expr_create_string(char* str){
    struct expr* new_expr = expr_create(EXPR_STRING, NULL, NULL);
    new_expr->string_literal = strcopy(str);
    
    printf("%s: done. string: %s \n",__func__, str); 
    
    return new_expr;
}

static void expr_destory(struct expr* target_expr){
    if(target_expr == NULL) return;

    expr_destory(target_expr->left);
    expr_destory(target_expr->right);

    if(target_expr->name) free((char*)target_expr->name);
    if(target_expr->string_literal) free((char*)target_expr->string_literal);
    
    free(target_expr);

    printf("%s: done.\n",__func__); 
}


char* strcopy(const char* name){
    if(name == NULL) {
        return NULL;
    }
    size_t len = strlen(name);
    char* new_str = malloc(sizeof(char) *len);
    strncpy(new_str, name, len);

    return new_str;
}
