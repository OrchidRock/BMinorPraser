#ifndef __SYMTABLE_H__
#define __SYMTABLE_H__

#include<stdio.h>
#include<stdlib.h>

typedef enum{
    SYMBOL_LOCAL,
    SYMBOL_PARAM,
    SYMBOL_GLOBAL
}symbol_t;

struct symbol {
    symbol_t kind;
    struct type* type;
    char* name;
    int which;
};


struct symbol * symbol_create(symbol_t kind, 
                struct type* type, 
                char* name);
void symbol_destory(struct symbol*);

const char* symbol_asmgen(struct symbol*);

void scope_enter();
void scope_exit();
int scope_level();

void scope_bind(const char *name, struct symbol *sym);
struct symbol* scope_lookup(const char* name);
struct symbol* scope_lookup_current(const char* name);
int scope_items_count();


#endif /* __SYMTABLE_H__ */
