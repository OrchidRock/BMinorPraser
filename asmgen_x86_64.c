
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include "asmgen_x86_64.h"
#include "symtable.h"

#define SCRATCH_REGS_NUMBER 7

static int scratch_alloc();
static void scratch_free(int t);
static const char* scratch_name(int t);

static int label_create();
static const char* label_name(int label);

struct ScratchReg{
    int r;
    char name[8];
    bool inuse;
};

static struct ScratchReg scratch_reg_table[SCRATCH_REGS_NUMBER]  = {
    {0, "%rbx", false},
    {1, "%r10", false},
    {2, "%r11", false},
    {3, "%r12", false},
    {4, "%r13", false},
    {5, "%r14", false},
    {6, "%r15", false},
};

static char argument_args_table[6][8] = {
    "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9",  
};

static int scratch_alloc(){
    for(int i =0; i < SCRATCH_REGS_NUMBER; i++){
        if(!scratch_reg_table[i].inuse){
            scratch_reg_table[i].inuse = true;
            return i;
        }
    }
    printf("error: there is not have a available scratch register.\n");
    return -1;
}

static const char* scratch_name(int r){
    if(r<0 || r>=SCRATCH_REGS_NUMBER){
        return "??";
    }else{
        return scratch_reg_table[r].name;
    }
}

static void scratch_free(int r){
    if(r>=0 && r < SCRATCH_REGS_NUMBER){
        scratch_reg_table[r].inuse = false;
    }
}



void expr_asmgen(struct expr* target_expr){
    if(target_expr == NULL) return ;
    
    int else_label, done_label;
    
    static int expr_arg_index = 0; 
    switch(target_expr->kind){
        case EXPR_NAME:
            target_expr->reg = scratch_alloc();
            printf("    MOVQ %s, %s\n", symbol_asmgen(target_expr->symbol),
                            scratch_name(target_expr->reg));
                        
            break;
        case EXPR_CHAR:
        case EXPR_NUMBER:
            target_expr->reg = scratch_alloc();
            printf("    MOVQ $%d, %s\n", target_expr->integer_value,
                           scratch_name(target_expr->reg));
            //printf("%d", target_expr->integer_value);

            break;

        case EXPR_STRING:
            printf("%s", target_expr->string_literal);
            break;
        
        case EXPR_MOD:
                expr_asmgen(target_expr->left);
                expr_asmgen(target_expr->right);
                printf("    MOVQ %s, %%rax\n", scratch_name(target_expr->left->reg));
                printf("    IDIVQ %s\n", scratch_name(target_expr->right->reg));
                printf("    MOVQ %%rdx, %s\n", scratch_name(target_expr->left->reg));
                target_expr->reg = target_expr->left->reg;
                scratch_free(target_expr->right->reg);

                break;
        case EXPR_DIV:
                expr_asmgen(target_expr->left);
                expr_asmgen(target_expr->right);
                printf("    MOVQ %s, %%rax\n", scratch_name(target_expr->left->reg));
                printf("    IDIV %s\n", scratch_name(target_expr->right->reg));
                printf("    MOVQ %%rax, %s\n", scratch_name(target_expr->left->reg));
                target_expr->reg = target_expr->left->reg;
                scratch_free(target_expr->right->reg);

                break;

        case EXPR_MUL:
                expr_asmgen(target_expr->left);
                expr_asmgen(target_expr->right);
                printf("    MOVQ %s, %%rax\n", scratch_name(target_expr->right->reg));
                printf("    IMULQ %s\n", scratch_name(target_expr->left->reg));
                printf("    MOVQ %%rax, %s\n", scratch_name(target_expr->right->reg));
                target_expr->reg = target_expr->right->reg;
                scratch_free(target_expr->left->reg);

                break;
            
        case EXPR_MINUS:
                expr_asmgen(target_expr->left);
                expr_asmgen(target_expr->right);
                printf("    SUBQ %s, %s\n", scratch_name(target_expr->right->reg),
                            scratch_name(target_expr->left->reg));
                target_expr->reg = target_expr->left->reg;
                scratch_free(target_expr->right->reg);

                break;
        case EXPR_ADD:
                expr_asmgen(target_expr->left);
                expr_asmgen(target_expr->right);
                printf("    ADDQ %s, %s\n", scratch_name(target_expr->left->reg),
                            scratch_name(target_expr->right->reg));
                target_expr->reg = target_expr->right->reg;
                scratch_free(target_expr->left->reg);

                break;
        
        case EXPR_AND:
                expr_asmgen(target_expr->left);
                expr_asmgen(target_expr->right);
                printf("    ANDQ %s, %s\n", scratch_name(target_expr->left->reg),
                           scratch_name(target_expr->right->reg));
                target_expr->reg = target_expr->right->reg;
                scratch_free(target_expr->left->reg);

                break;
        
        case EXPR_OR:
                expr_asmgen(target_expr->left);
                expr_asmgen(target_expr->right);
                printf("    ORQ %s, %s\n", scratch_name(target_expr->left->reg),
                           scratch_name(target_expr->right->reg));
                target_expr->reg = target_expr->right->reg;
                scratch_free(target_expr->left->reg);
                break;

        case EXPR_ASSIGN:
            expr_asmgen(target_expr->right);
            //expr_asmgen()
            printf("    MOVQ %s, %s\n", scratch_name(target_expr->right->reg),
                            symbol_asmgen(target_expr->left->symbol));
            target_expr->reg = target_expr->right->reg;
            scratch_free(target_expr->reg);
            break;
        case EXPR_ARG:
            expr_asmgen(target_expr->left);
            printf("    MOVQ %s, %s\n", scratch_name(target_expr->left->reg),
                            argument_args_table[expr_arg_index++]);
            scratch_free(target_expr->left->reg);
            expr_asmgen(target_expr->right);
            break;
        case EXPR_CALL:
            expr_arg_index = 0;
            expr_asmgen(target_expr->right);
            printf("    call %s\n", symbol_asmgen(target_expr->left->symbol));
            scratch_free(target_expr->right->reg);
            target_expr->reg = scratch_alloc();
            printf("    MOVQ %%rax, %s\n", scratch_name(target_expr->reg));
            break;
        
        case EXPR_NOTEQUAL:
            expr_asmgen(target_expr->left);
            expr_asmgen(target_expr->right);
            done_label = label_create();
            else_label = label_create();
            printf("    CMPQ %s, %s\n", scratch_name(target_expr->left->reg), 
                            scratch_name(target_expr->right->reg));
            printf("    JNE %s\n", label_name(else_label)); /*true*/
            printf("    MOVQ $0, %%rax\n");
            printf("    JMP %s\n", label_name(done_label));
            printf("%s:\n", label_name(else_label));
            printf("    MOVQ $1, %%rax\n");
            printf("%s:\n", label_name(done_label));
            scratch_free(target_expr->left->reg);
            scratch_free(target_expr->right->reg);
            break;
        case EXPR_EQUAL:
            expr_asmgen(target_expr->left);
            expr_asmgen(target_expr->right);
            done_label = label_create();
            else_label = label_create();
            printf("    CMPQ %s, %s\n", scratch_name(target_expr->left->reg), 
                            scratch_name(target_expr->right->reg));
            printf("    JE %s\n", label_name(else_label)); /*true*/
            printf("    MOVQ $0, %%rax\n");
            printf("    JMP %s\n", label_name(done_label));
            printf("%s:\n", label_name(else_label));
            printf("    MOVQ $1, %%rax\n");
            printf("%s:\n", label_name(done_label));
            scratch_free(target_expr->left->reg);
            scratch_free(target_expr->right->reg);
            break;
        case EXPR_LESS:
            expr_asmgen(target_expr->left);
            expr_asmgen(target_expr->right);
            done_label = label_create();
            else_label = label_create();
            printf("    CMPQ %s, %s\n", scratch_name(target_expr->right->reg), 
                            scratch_name(target_expr->left->reg));
            printf("    JL %s\n", label_name(else_label)); /*true*/
            printf("    MOVQ $0, %%rax\n");
            printf("    JMP %s\n", label_name(done_label));
            printf("%s:\n", label_name(else_label));
            printf("    MOVQ $1, %%rax\n");
            printf("%s:\n", label_name(done_label));
            scratch_free(target_expr->left->reg);
            scratch_free(target_expr->right->reg);
            break;
        case EXPR_LESSEQU:
            expr_asmgen(target_expr->left);
            expr_asmgen(target_expr->right);
            done_label = label_create();
            else_label = label_create();
            printf("    CMPQ %s, %s\n", scratch_name(target_expr->right->reg), 
                            scratch_name(target_expr->left->reg));
            printf("    JLE %s\n", label_name(else_label)); /*true*/
            printf("    MOVQ $0, %%rax\n");
            printf("    JMP %s\n", label_name(done_label));
            printf("%s:\n", label_name(else_label));
            printf("    MOVQ $1, %%rax\n");
            printf("%s:\n", label_name(done_label));
            scratch_free(target_expr->left->reg);
            scratch_free(target_expr->right->reg);
            break;
        case EXPR_GREATER:
            expr_asmgen(target_expr->left);
            expr_asmgen(target_expr->right);
            done_label = label_create();
            else_label = label_create();
            printf("    CMPQ %s, %s\n", scratch_name(target_expr->right->reg), 
                            scratch_name(target_expr->left->reg));
            printf("    JG %s\n", label_name(else_label)); /*true*/
            printf("    MOVQ $0, %%rax\n");
            printf("    JMP %s\n", label_name(done_label));
            printf("%s:\n", label_name(else_label));
            printf("    MOVQ $1, %%rax\n");
            printf("%s:\n", label_name(done_label));
            scratch_free(target_expr->left->reg);
            scratch_free(target_expr->right->reg);
            break;
        case EXPR_GREATEREQU:
            expr_asmgen(target_expr->left);
            expr_asmgen(target_expr->right);
            done_label = label_create();
            else_label = label_create();
            printf("    CMPQ %s, %s\n", scratch_name(target_expr->right->reg), 
                            scratch_name(target_expr->left->reg));
            printf("    JGE %s\n", label_name(else_label)); /*true*/
            printf("    MOVQ $0, %%rax\n");
            printf("    JMP %s\n", label_name(done_label));
            printf("%s:\n", label_name(else_label));
            printf("    MOVQ $1, %%rax\n");
            printf("%s:\n", label_name(done_label));
            scratch_free(target_expr->left->reg);
            scratch_free(target_expr->right->reg);
            break;
        case EXPR_INC:
            expr_asmgen(target_expr->left);
            printf("    ADDQ $1, %s\n", scratch_name(target_expr->left->reg));
            printf("    MOVQ %s, %s\n", scratch_name(target_expr->left->reg),
                            symbol_asmgen(target_expr->left->symbol));
            scratch_free(target_expr->left->reg);
            break;
        default:
            break;
    }
}


void decl_asmgen(struct decl* d){
    if(d == NULL) return;
    if(d->symbol == NULL) return;
    type_t kind = d->symbol->type->kind;
    if(d->symbol->kind == SYMBOL_GLOBAL){
        printf(".globl %s\n", d->name);
    }else{ /* in the block, such as function|if_else|for. */
        return;
    }

    if(kind == TYPE_INTEGER){
        printf(".data\n");
        printf(".type %s, @object\n", d->name);
        printf("%s:\n", d->name);
        printf("    .long ");
        expr_print_to_fd(d->value, stdout);
        printf("\n");
    }else if (kind == TYPE_STRING){
        printf(".data\n");
        printf(".type %s, @object\n", d->name);
        printf("%s:\n", d->name);
        printf("    .string ");
        expr_print_to_fd(d->value, stdout);
        printf("\n");
    }else if(kind == TYPE_FUNCTION){
        printf(".text\n");
        printf(".type %s, @function\n", d->name);
        printf("%s:\n", d->name);
        //stmt_asmgen(d->code);
    }else if(kind == TYPE_CHARACTER){
        printf(".data\n");
        printf(".type %s, @object\n", d->name);
        printf("%s:\n", d->name);
        printf("    .byte ");
        expr_print_to_fd(d->value, stdout);
        printf("\n");
    }else{
    
    }
}

void stmt_asmgen(struct stmt* st, struct type* tp){
    if(st == NULL) return;
    
    int local_variable_count, function_arg_count;
    struct decl* d = st->decl;
    static int top_label_for, done_label_for;
    static int false_label_if, done_label_if;
    static bool alloc_stack = false;
    switch(st->kind){
        case STMT_DECL:
            expr_asmgen(d->value);
            if(d->value){
                printf("    MOVQ %s, %s\n", scratch_name(d->value->reg),
                    symbol_asmgen(d->symbol));
                scratch_free(d->value->reg);
            }else{
                //
            }
            break;
        case STMT_EXPR:
                if(st->expr){
                    expr_asmgen(st->expr);
                }
                break;
        case STMT_BLOCK:
                /* leaf ?*/
                if(tp == NULL){
                    stmt_asmgen(st->body, NULL);
                    break;
                }
                local_variable_count = scope_items_count();
                if(tp->params){
                    function_arg_count = param_list_count(tp->params);
                }else{
                    function_arg_count = 0;
                }

                if(local_variable_count == 0){ // leaf
                    //
                }else{
                    printf("    PUSHQ %%rbp\n");
                    printf("    MOVQ %%rsp, %%rbp\n");
                    printf("    SUBQ $%ld, %%rsp\n", sizeof(long)*local_variable_count);
                    alloc_stack = true;
                    //printf("    SUBQ $32, %%rsp\n");
                    for(int i=0; i<function_arg_count;i++){
                        printf("    MOVQ %s, -%ld(%%rbp)\n", argument_args_table[i],
                                        (i+1)*sizeof(long));
                    }
                }
                stmt_asmgen(st->body, NULL);
                alloc_stack = false; 
                break;
        case STMT_WHILE:
                
                printf("\n");
                
                top_label_for = label_create();
                done_label_for = label_create();
                printf("%s:\n", label_name(top_label_for)); 
                
                expr_asmgen(st->expr);
                printf("    CMPQ $0, %%rax\n");
                printf("    JE %s\n", label_name(done_label_for));
                
                if(st->body){
                    stmt_asmgen(st->body, NULL);
                }

                printf("    JMP %s\n", label_name(top_label_for));
                
                printf("%s:\n", label_name(done_label_for));
                break;
        case STMT_IF_ELSE:
                false_label_if = label_create();
                done_label_if = label_create();
                printf("\n");
                expr_asmgen(st->expr);
                printf("    CMPQ $0, %%rax\n");
                if(st->else_body){
                    printf("    JE %s\n", label_name(false_label_if));
                }else{
                    printf("    JE %s\n", label_name(done_label_if));
                }
                stmt_asmgen(st->body, NULL);
                    
                if(st->else_body){
                    printf("    JMP %s\n", label_name(done_label_if));
                    printf("%s:\n", label_name(false_label_if));
                    stmt_asmgen(st->else_body, NULL);
                } 

                printf("%s:\n", label_name(done_label_if));
                break;
        case STMT_FOR:
                printf("\n");
                expr_asmgen(st->init_expr);
                top_label_for = label_create();
                done_label_for = label_create();
                printf("%s:\n", label_name(top_label_for)); 
                expr_asmgen(st->expr);
                printf("    CMPQ $0, %%rax\n");
                printf("    JE %s\n", label_name(done_label_for));
                
                if(st->body){
                    stmt_asmgen(st->body, NULL);
                }
                    
                expr_asmgen(st->next_expr);
                printf("    JMP %s\n", label_name(top_label_for));
                
                printf("%s:\n", label_name(done_label_for));

                break;
        case STMT_RETURN:
            if(st->expr){
                expr_asmgen(st->expr);
                printf("    MOVQ %s, %%rax\n", scratch_name(st->expr->reg));
                scratch_free(st->expr->reg);
            }
            if(alloc_stack){
                printf("    leave\n");
            }else{
                //printf("    POPQ %%rbp\n");
            }
            printf("    RET\n");
            break;
        case STMT_PRINT:
            printf("    leaq ");
            expr_print_to_fd(st->expr, stdout);
            printf("(%%rip), %%rdi\n"); 

            expr_asmgen(st->next_expr);
            printf("    MOVQ %s, %%rax\n", scratch_name(st->next_expr->reg));
            printf("    MOVQ %%rax, %%rsi\n");
            printf("    MOVQ $0, %%rax\n");
            printf("    call printf@PLT\n");
            scratch_free(st->next_expr->reg);
            break;
        default:
              break;  
    }

    stmt_asmgen(st->next, NULL);
}



static int label_create(){
    static int label_count = 0;
    return label_count ++;
}

static const char* label_name(int num){
    static char label_name_buf[10] = {0};
    memset(label_name_buf,0, sizeof(label_name_buf));
    snprintf(label_name_buf, sizeof(label_name_buf), ".L%d", num);
    return label_name_buf;
}

