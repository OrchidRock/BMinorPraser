
#include<uthash.h>

#include "symtable.h"
#include "ast.h"


static  int curr_scope_level = 1; /* Default: GLOBAL level.*/

struct SymbolHashTable {
    const char* name; /* key */
    struct symbol* symbol;

    UT_hash_handle hh; /* uthash handler. */
};

struct ScopeStackNode {
    struct SymbolHashTable* symbol_table;
    struct ScopeStackNode* next;
};

static struct ScopeStackNode* scope_stack = NULL;

static void hashtable_destory(struct SymbolHashTable* head){
    struct SymbolHashTable *current, *tmp;

    HASH_ITER(hh, head, current, tmp) {
        //printf("Free item [%s] in scope %d \n", current->name, 
        //                curr_scope_level);
        
        HASH_DEL(head, current);
        symbol_destory(current->symbol);
        free(current);
    }
    //printf("hashtable len: %d after destory scope %d.\n", HASH_COUNT(head),
    //                curr_scope_level);
}

struct symbol * symbol_create(symbol_t kind, 
                struct type* type, 
                char* name){
    struct symbol *new_symbol = malloc(sizeof(struct symbol));
    new_symbol->kind = kind;
    new_symbol->type = type;
    new_symbol->name = name;
    //new_symbol->which = which;
    return new_symbol;
}

void symbol_destory(struct symbol* target_symbol){
    if(target_symbol == NULL) return;

    free(target_symbol);
}


void scope_enter(){
    if(scope_stack != NULL){
        curr_scope_level = curr_scope_level + 1;
    }
    struct ScopeStackNode* new_scope = malloc(sizeof(struct ScopeStackNode));
    new_scope->symbol_table = NULL;
    new_scope->next = scope_stack;
    scope_stack = new_scope;

    //printf("****  %s %d\n", __func__, curr_scope_level);
}

void scope_exit(){
    if(scope_stack == NULL) return;
    
    struct ScopeStackNode* new_stack_top = scope_stack->next;
    hashtable_destory(scope_stack->symbol_table);
    free(scope_stack);
    scope_stack = new_stack_top;

    curr_scope_level = curr_scope_level - 1;
    
    //printf("**** %s %d\n", __func__, curr_scope_level+1);
}

int scope_level(){
    return curr_scope_level;
}

void scope_bind(const char *name, struct symbol *sym){
    if(scope_stack == NULL) {
        scope_stack = malloc(sizeof(struct ScopeStackNode));
        scope_stack->symbol_table = NULL; 
        scope_stack->next = NULL;
    }

    if(scope_stack->symbol_table && scope_lookup_current(name)){
        printf("error: '%s' has been declared.\n", name);
        
        /*Note: don't forget to free the symbol object.*/
        free(sym);
        return;
    }

    struct SymbolHashTable* new_item = malloc(sizeof(struct SymbolHashTable));
    new_item->name = name;
    new_item->symbol = sym;
    HASH_ADD_STR(scope_stack->symbol_table, name, new_item);
    
    new_item->symbol->which = HASH_COUNT(scope_stack->symbol_table);
    //printf("%s %s which: %d\n", __func__, name, new_item->symbol->which);    
    /*
    printf("Add new item [%s](which: %d) into level %d (%d).\n",
                    name,
                    sym->which,
                    curr_scope_level, 
                    HASH_COUNT(scope_stack->symbol_table));
    */
}

struct symbol* scope_lookup(const char* name){
    struct SymbolHashTable* target_item = NULL;
    struct ScopeStackNode* current = scope_stack; 
    
    while(current){
        HASH_FIND_STR(current->symbol_table, name, target_item);
        if(target_item){
            break;
        }
        current = current->next;
    }
    
    if(target_item){
        return target_item->symbol;
    }else{
        return NULL;
    }
}

struct symbol* scope_lookup_current(const char* name){
    struct SymbolHashTable* target_item = NULL;
    struct ScopeStackNode* current = scope_stack; 
    HASH_FIND_STR(current->symbol_table, name, target_item);
    if(target_item){
        return target_item->symbol;
    }
    return NULL;
}


const char* symbol_asmgen(struct symbol* sym){
    static char bp_index_temp[20] = {0};
    memset(bp_index_temp,0, sizeof(bp_index_temp));
    if(sym->kind == SYMBOL_GLOBAL){
        if(sym->type->kind == TYPE_FUNCTION){
            return sym->name;
        }else{
            snprintf(bp_index_temp,sizeof(bp_index_temp), "%s(%%rip)", sym->name);
        }
        //printf("%s(%%rip)", sym->name);
        //return sym->name;
    }
    else{
        int shift = sym->which * sizeof(long);
        snprintf(bp_index_temp, sizeof(bp_index_temp), "-%d(%%rbp)", shift);
        //printf("-%d(%%rbp)", shift);
    }
    return bp_index_temp;
}


int scope_items_count(){
    if(scope_stack == NULL) return 0;
    if(scope_stack->symbol_table == NULL) return 0;
    return HASH_COUNT(scope_stack->symbol_table);
}
