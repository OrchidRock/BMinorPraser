#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>

#include "ast.h"

static void type_destory(struct type*);
static void param_list_destory(struct param_list*);
static void expr_destory(struct  expr*);
static void stmt_destory(struct stmt*);

/* type checking. */
static struct type* expr_typecheck(struct expr*);
static void decl_typecheck(struct decl*);
static void stmt_typecheck(struct stmt*);
static bool type_equal(struct type*, struct type*);

/* print */
static void type_print(struct type*);
static void expr_print(struct expr*);


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
    d->symbol = NULL;

    //printf("decl_create: %s done.\n", name); 

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
    
    //printf("%s: done.\n",__func__); 
}

void decl_resolve(struct decl *d){
    if(!d) return;
    symbol_t kind = scope_level() > 1 ? SYMBOL_LOCAL : SYMBOL_GLOBAL;
    d->symbol = symbol_create(kind, d->type, d->name);
    
    expr_resolve(d->value);
    
    scope_bind(d->name, d->symbol);

    if(d->code) { /* function */
        scope_enter();
        param_list_resolve(d->type->params);
        stmt_resolve(d->code);
        scope_exit();
    }
    
    decl_typecheck(d); /**/
    
    decl_resolve(d->next);
}

static void decl_typecheck(struct decl* target_decl){
    if(target_decl == NULL) return;

    if(target_decl->value){ /* expression */
        struct type* t;
        t = expr_typecheck(target_decl->value);
        if(!type_equal(t, target_decl->symbol->type)){
            printf("error: "); 
            expr_print(target_decl->value),
            printf("cann't be declared to type:"); 
            type_print(target_decl->symbol->type);
            printf(".\n"); 
        }
        type_destory(t);
    }
    
    if(target_decl->code){
        //stmt_typecheck(target_decl->code);
    }

} 


struct type* type_create(type_t kind, struct type* stype, struct param_list* pl){
    struct type* t = malloc(sizeof(struct type));
    t->kind = kind;
    t->subtype = stype;
    t->params = pl;
    
    //printf("%s: %d done.\n",__func__, kind); 

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

static void type_print(struct type* target_type){
    if(target_type == NULL) return;
    switch(target_type->kind){
        case TYPE_VOID: printf(" void "); break;
        case TYPE_ARRAY: printf(" array "); break;
        case TYPE_STRING: printf(" string "); break;
        case TYPE_BOOLEAN: printf(" boolean "); break;
        case TYPE_INTEGER: printf(" integer "); break;
        case TYPE_FUNCTION: printf(" function "); break;
        case TYPE_CHARACTER: printf(" char "); break;
        default: printf(" UNKNOWN "); break;
    }
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

void param_list_resolve(struct param_list* pl){
    if(pl == NULL) return;
    pl->symbol = symbol_create(SYMBOL_LOCAL, pl->type, pl->name);
    scope_bind(pl->name, pl->symbol);

    param_list_resolve(pl->next);
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

    //printf("%s: done. kind: %d \n",__func__, kind); 
    
    return new_stmt;
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
    //printf("%s: done.\n",__func__); 
}
void stmt_resolve(struct stmt* target_stmt){
    if(target_stmt == NULL) return;

    decl_resolve(target_stmt->decl);
    
    expr_resolve(target_stmt->init_expr);
    expr_resolve(target_stmt->expr);
    expr_resolve(target_stmt->next_expr);

    if(target_stmt->body){
        scope_enter();
        stmt_resolve(target_stmt->body);
        scope_exit();
    }
    if(target_stmt->else_body){
        scope_enter();
        stmt_resolve(target_stmt->else_body);
        scope_exit();
    }
    
    stmt_typecheck(target_stmt);

    stmt_resolve(target_stmt->next);
}

void stmt_typecheck(struct stmt *target_stmt){
    if(target_stmt == NULL) return;

    struct type * t;
    switch(target_stmt->kind){
            case STMT_EXPR:
                t = expr_typecheck(target_stmt->expr);
                type_destory(t);
                break;
            case STMT_FOR:
                t = expr_typecheck(target_stmt->expr);
                if(t->kind != TYPE_BOOLEAN){
                    printf("error: ");
                    type_print(t);
                    expr_print(target_stmt->expr);
                    printf("isn't a boolean expression at for stmt.\n");
                }   
                type_destory(t);
                
                t = expr_typecheck(target_stmt->init_expr);
                type_destory(t);
                t = expr_typecheck(target_stmt->next_expr);
                type_destory(t);
                
                stmt_typecheck(target_stmt->body); 

                break;
            case STMT_RETURN:
                break;
            case STMT_PRINT:
                break;     
            case STMT_IF_ELSE:
                t = expr_typecheck(target_stmt->expr);
                if(t->kind != TYPE_BOOLEAN){
                    printf("error:");
                    type_print(t);
                    expr_print(target_stmt->expr);
                    printf("isn't a boolean expression at if_else stmt.\n");
                }   
                type_destory(t);
                stmt_typecheck(target_stmt->body);
                stmt_typecheck(target_stmt->else_body);
                break;
            case STMT_BLOCK:
                stmt_typecheck(target_stmt->body);
                break;
            default:
                break;
    }
    if(target_stmt->body){
        //stmt_typecheck(target_stmt->body);
    }
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
    new_expr->symbol = NULL;

    //printf("%s: done. kind: %d \n",__func__, kind); 
    
    return new_expr;
}

struct expr* expr_create_name(char* str){
    struct expr* new_expr = expr_create(EXPR_NAME, NULL, NULL);
    new_expr->name = str;
    
    //printf("%s: done. name: %s \n",__func__, str); 
    
    return new_expr;
}
struct expr* expr_create_number(int number){
    struct expr* new_expr = expr_create(EXPR_NUMBER, NULL, NULL);
    new_expr->integer_value = number;
    
    //printf("%s: done. number: %d \n",__func__, number); 
    
    return new_expr;
}

struct expr* expr_create_string(char* str){
    struct expr* new_expr = expr_create(EXPR_STRING, NULL, NULL);
    new_expr->string_literal = strcopy(str);
    
    //printf("%s: done. string: %s \n",__func__, str); 
    
    return new_expr;
}
struct expr* expr_create_char(int ch){
    struct expr* new_expr = expr_create(EXPR_CHAR, NULL, NULL);
    new_expr->integer_value = ch;
    return new_expr; 
}


static void expr_destory(struct expr* target_expr){
    if(target_expr == NULL) return;

    expr_destory(target_expr->left);
    expr_destory(target_expr->right);

    if(target_expr->name) free((char*)target_expr->name);
    if(target_expr->string_literal) free((char*)target_expr->string_literal);
    
    free(target_expr);

    //printf("%s: done.\n",__func__); 
}

void expr_resolve(struct expr* e){
    if(!e) return;
    if(e->kind == EXPR_NAME) {
        e->symbol = scope_lookup(e->name);
        if(e->symbol){
            //printf("Find Symbol [%s] has type [%d]. \n", e->symbol->name,
            //                e->symbol->type->kind);
        }else{
            printf("error: symbol '%s' hasn't been declared.\n", e->name);
        }
    } else {
        expr_resolve(e->left);
        expr_resolve(e->right);       
    }

    //expr_typecheck(e);
}


bool type_equal(struct type *a, struct type *b){
    if(a == NULL || b == NULL) {
        fprintf(stderr, "%s error type  is null\n", __func__);
    }
    if(a->kind == b->kind){
        if(a->kind == TYPE_ARRAY){
            if(a->subtype->kind == b->subtype->kind) return true;
            else return false;
        }else if(a->kind == TYPE_FUNCTION){
            if(type_equal(a->subtype, b->subtype)){
                struct param_list* a_tmp, *b_tmp;
                a_tmp = a->params;
                b_tmp = b->params;
                while(a_tmp && b_tmp){
                    if(a_tmp->type->kind != b_tmp->type->kind){
                        return false;
                    }
                    a_tmp = a_tmp->next;
                    b_tmp = b_tmp->next;
                }
                if(a_tmp || b_tmp){
                    return false;
                }else{
                    return true;
                }
            }
        }else{
            return true;
        }
    }else if(a->kind == TYPE_CHARACTER && b->kind == TYPE_INTEGER ||
             a->kind == TYPE_INTEGER && b->kind == TYPE_CHARACTER){
        return true;
    }else{
        return false;
    }
}

struct type* expr_typecheck(struct expr *target_expr){
    if(target_expr == NULL) return NULL;

    struct type* lt = expr_typecheck(target_expr->left);
    struct type* rt = expr_typecheck(target_expr->right);
    
    struct type *result = NULL;

    switch(target_expr->kind){
        case EXPR_NUMBER:
            result = type_create(TYPE_INTEGER, NULL, NULL);
            break;
        case EXPR_STRING:
            result = type_create(TYPE_STRING, NULL, NULL);
            break;
        case EXPR_NAME:
            if(target_expr->symbol){
                result = type_create(target_expr->symbol->type->kind,NULL, NULL);
            }else{
                result = type_create(TYPE_VOID, NULL, NULL);
            }
            break;
        case EXPR_CHAR:
            result = type_create(TYPE_CHARACTER, NULL, NULL);
            break; 
        case EXPR_CALL:
            break;
        
        case EXPR_SUBCRIPT:
            if(rt->kind != TYPE_INTEGER){
                fprintf(stderr, "subscrip only by interger.\n");
            }
            result = type_create(lt->kind, NULL, NULL);
            break;
        case EXPR_MOD:
        case EXPR_DIV:
        case EXPR_MINUS:
        case EXPR_MUL:
        case EXPR_ADD:
            if(lt->kind != TYPE_INTEGER || rt->kind != TYPE_INTEGER){
                printf("error: cannot (+|-|*|/|%%) a");
                type_print(lt);
                expr_print(target_expr->left);
                printf("to a");
                type_print(rt);
                expr_print(target_expr->right);
                printf(".\n");
            }
            result = type_create(TYPE_INTEGER, NULL, NULL);
            break;
        
        case EXPR_ASSIGN:
            if(!type_equal(lt, rt)){
                printf("error: cannot assign a ");
                type_print(rt);
                printf("to");
                expr_print(target_expr->left);
                printf(".\n");
            }
            result = type_create(lt->kind, NULL, NULL);
            break;

        case EXPR_GREATER:
        case EXPR_GREATEREQU:
        case EXPR_LESS:
        case EXPR_LESSEQU:
        case EXPR_EQUAL:
        case EXPR_NOTEQUAL:
            if(!type_equal(lt, rt)){
                printf("error: cannot compare a");
                type_print(lt);
                expr_print(target_expr->left);
                printf("to an");
                type_print(rt);
                expr_print(target_expr->right);
                printf(".\n");     
            }
            if(lt->kind  == TYPE_VOID ||
                lt->kind == TYPE_ARRAY ||
                lt->kind == TYPE_FUNCTION){
                
            }
            result = type_create(TYPE_BOOLEAN, NULL, NULL);
            break;
        default:
            break;
    }


    type_destory(lt);
    type_destory(rt);

    return result;
}


void expr_print(struct expr* target_expr){
    if(target_expr == NULL) return;
    
    switch(target_expr->kind){
        case EXPR_NAME: printf(" %s ", target_expr->name); break;
        case EXPR_NUMBER: printf(" %d ", target_expr->integer_value); break;
        case EXPR_STRING: printf(" %s ", target_expr->string_literal); break;
        case EXPR_ADD:
                expr_print(target_expr->left);
                printf(" + "); 
                expr_print(target_expr->right);
                break;
        case EXPR_MUL: printf("-"); break;
        case EXPR_SUBCRIPT:
            printf(" [] ");
            break;
        default: printf(" expr "); break;
    }
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


