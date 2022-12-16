
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include "asmgen_llvm.h"
#include "symtable.h"

static int ssa_count = 0;
static int function_arg_count;

int expr_reg_count(struct expr*);
int stmt_reg_count(struct stmt*);

static int bytes_count(const char* str){
    if(str == NULL) return 0;
    const char* tmp = str;
    int result = 0;
    while(*tmp){
        if(*tmp == '\\'){
            tmp += 2;
        }else{
            tmp += 1;
        }
        result ++;
    }

    return result;
}

static char* format_string(const char* str){
    if(str == NULL) return NULL;

    static char buffer[1024] = {0};
    const char* tmp =  str;
    char* buf_ptr  = buffer;
    *buf_ptr++ = '\"';
    while(*tmp){
        if(*tmp == '\\'){
            if(*(++tmp) == 'n'){
                *buf_ptr = '\\';
                buf_ptr++;
                *buf_ptr = '0';
                buf_ptr++;
                *buf_ptr = 'A';
            }else{
                
            }
        }else if(*tmp == '\"'){
            tmp++;
            continue;
        }else{
            *buf_ptr = *tmp;
        }
        buf_ptr ++;
        tmp++;
    }
    *buf_ptr++ = '\\';
    *buf_ptr++ = '0';
    *buf_ptr++ = '0';
    
    *buf_ptr++ = '\"'; 
    *buf_ptr = '\0';

    return buffer;
}

static void type_asmgen_llvm(struct type* tp){
    if(tp == NULL) return;
    switch(tp->kind){
        case TYPE_INTEGER:
            printf("i32");
            break;
        case TYPE_CHARACTER:
            printf("i8");
            break;
        case TYPE_FUNCTION:
            type_asmgen_llvm(tp->subtype);
            break;
        default:
            printf("i32");
            break;
    }
}

static void param_list_asmgen_llvm(struct param_list* pl){
    if(pl==NULL){ 
        return;
    }
    struct symbol* sym = pl->symbol;
    if(sym == NULL){
        return;
    }

    type_asmgen_llvm(sym->type);
    printf(" %%");
    printf("%d", (sym->which)-1);
    if(pl->next){
        printf(", ");
        param_list_asmgen_llvm(pl->next);
    }
}

static void symbol_asmgen_llvm(struct symbol* sym){
    if(sym == NULL) return;
    
    if(sym->kind == SYMBOL_GLOBAL){
        if(sym->type->kind == TYPE_STRING){
            printf("    %%%d = load i8*, i8** @%s, align 8\n",ssa_count++, sym->name);
        }else{
            printf("    %%%d = load i32, i32* @%s, align 4\n",ssa_count++, sym->name);
        }
    }else{
        printf("    %%%d = load i32, i32* %%%d, align 4\n", 
                        ssa_count++, sym->which+function_arg_count+1);
    }

}

void decl_asmgen_llvm(struct decl* d){
    if(d == NULL) return;
    if(d->symbol == NULL) return;
    type_t kind = d->symbol->type->kind;
    
    int len;
    
    static bool printf_declare_exists = false;

    if(d->symbol->kind == SYMBOL_GLOBAL){
        if(!printf_declare_exists){
            printf("declare dso_local i32 @printf(i8*,...)\n");
            printf_declare_exists = true;
        }
    }else{ /* in the block, such as function|if_else|for. */
        return;
    }

    if(kind == TYPE_INTEGER){
        printf("@%s = dso_local global ", d->name);
        type_asmgen_llvm(d->type);
        printf(" ");
        expr_print_to_fd(d->value, stdout);
        printf(", align 4\n");
    }else if (kind == TYPE_STRING){
        
        if(d->value){
            len = bytes_count(d->value->string_literal);
            printf("@.%s = private unnamed_addr constant [%d x i8] c%s, align 1\n",
                    d->name, 
                    len-1,
                    format_string(d->value->string_literal));
            printf("@%s = dso_local global i8* getelementptr inbounds ([%d x i8], [%d x i8]* @.%s, i32 0, i32 0), align 8\n",
                    d->name,
                    len-1,
                    len-1,
                    d->name);
        }
    }else if(kind == TYPE_FUNCTION){
        printf("define dso_local ");
        type_asmgen_llvm(d->type);
        printf(" @%s", d->name);
        //stmt_asmgen(d->code);
    }else if(kind == TYPE_CHARACTER){
        printf("@%s = dso_local global ", d->name);
        type_asmgen_llvm(d->type);
        printf(" ");
        expr_print_to_fd(d->value, stdout);
        printf(", align 1\n");
    }else{
    
    }
}


void stmt_asmgen_llvm(struct stmt* st, struct type* tp){
    if(st == NULL) return;
    
    int ssa_count_tmp = ssa_count;
    int ssa_count_tmp2 ;
    int local_variable_count;
    struct decl* d = st->decl;
    static int top_label_for, done_label_for, else_label_for;
    static int false_label_if, done_label_if, true_label_if;
    
    switch(st->kind){
        case STMT_DECL:
            decl_asmgen_llvm(d);
            if(d->value) {
                ssa_count_tmp = expr_asmgen_llvm(d->value);
                printf("    store i32 %%%d, i32* %%%d, align 4\n", ssa_count_tmp, 
                            d->symbol->which+function_arg_count+1);
            }
            if(st->next && st->next->kind == STMT_WHILE){
                printf("    br label %%%d\n", ssa_count);
            }
            break;
        case STMT_EXPR:
                expr_asmgen_llvm(st->expr);
                if(st->next && st->next->kind == STMT_WHILE){
                    printf("    br label %%%d\n", ssa_count);
                }
                break;
        case STMT_BLOCK:
                /* leaf ?*/
                if(tp == NULL){
                    stmt_asmgen_llvm(st->body, NULL);
                    break;
                }
                printf("(");
                param_list_asmgen_llvm(tp->params);
                printf(") #0 {\n");
                
                local_variable_count = scope_items_count();
                function_arg_count = param_list_count(tp->params);
                printf("    %%%d = alloca ", function_arg_count+1);
                type_asmgen_llvm(tp->subtype);
                printf(", align 4\n");
                for(int i=function_arg_count+2; i<local_variable_count+function_arg_count+2;i++){
                    printf("    %%%d = alloca i32, align 4\n", i);
                }
                for(int i=0; i<function_arg_count; i++){
                    printf("    store i32 %%%d, i32* %%%d, align 4\n", i, i+function_arg_count+2);
                }

                ssa_count = local_variable_count + function_arg_count + 2;
                
                
                stmt_asmgen_llvm(st->body, NULL);

                printf("}\n");
                break;
        case STMT_WHILE:
                printf("\n");
                top_label_for = ssa_count++;
                printf("%d:\n", top_label_for);
                ssa_count_tmp = expr_asmgen_llvm(st->expr);
                else_label_for = ssa_count ++;
                done_label_for = ssa_count + stmt_reg_count(st->body);
                printf("    br i1 %%%d, label %%%d, label %%%d\n", ssa_count_tmp,else_label_for, done_label_for);
                printf("%d:\n", else_label_for);
                
                stmt_asmgen_llvm(st->body, NULL);
                
                printf("    br label %%%d\n", top_label_for);
                
                printf("%d:\n", done_label_for);
                ssa_count = done_label_for + 1;
                
                break;
        case STMT_IF_ELSE:
                printf("\n");
                ssa_count_tmp = expr_asmgen_llvm(st->expr);
                true_label_if = ssa_count++;
                //false_label_if = ssa_count++;
                
                if(st->else_body){
                    done_label_if = ssa_count + stmt_reg_count(st->body) + stmt_reg_count(st->else_body)+1;
                    false_label_if = ssa_count + stmt_reg_count(st->body);
                    printf("    br i1 %%%d, label %%%d, label %%%d\n", ssa_count_tmp, true_label_if, false_label_if);
                }else{
                    done_label_if = ssa_count + stmt_reg_count(st->body);
                    printf("    br i1 %%%d, label %%%d, label %%%d\n", ssa_count_tmp, true_label_if, done_label_if);
                }
                printf("%d:\n", true_label_if);
                stmt_asmgen_llvm(st->body, NULL);
                printf("    br label %%%d\n", done_label_if);

                if(st->else_body){
                    printf("%d:\n", false_label_if);
                    ssa_count = false_label_if + 1;
                    stmt_asmgen_llvm(st->else_body, NULL);
                    printf("    br label %%%d\n", done_label_if); 
                }else{
                    //printf("%d:\n", false_label_if);
                    //printf("    br label %%%d\n", done_label_if); 
                }
                if(st->next && st->next->kind != STMT_WHILE){
                    printf("%d:\n", done_label_if);
                    ssa_count = done_label_if + 1;
                }
                break;
        case STMT_FOR:
                expr_asmgen_llvm(st->init_expr);
                top_label_for = ssa_count++;
                printf("    br label %%%d\n", top_label_for);
                printf("%d:\n", top_label_for);
                ssa_count_tmp = expr_asmgen_llvm(st->expr);
                else_label_for = ssa_count++;
                done_label_for = ssa_count + stmt_reg_count(st->body) + expr_reg_count(st->next_expr);
               
                printf("    br i1 %%%d, label %%%d, label %%%d\n", ssa_count_tmp,else_label_for, done_label_for);
                printf("%d:\n", else_label_for);
                
                stmt_asmgen_llvm(st->body, NULL);    
                
                expr_asmgen_llvm(st->next_expr);
                printf("    br label  %%%d\n", top_label_for);
                 
                printf("%d:\n", done_label_for);
                ssa_count = done_label_for + 1;
                break;
        case STMT_RETURN:
            ssa_count_tmp = expr_asmgen_llvm(st->expr);
            //printf("    store i32 %%%d, i32* %%%d, align 4\n", ssa_count_tmp, 
            //                function_arg_count+1);
            printf("    ret  i32 %%%d\n", ssa_count_tmp);

            break;
        case STMT_PRINT:
            ssa_count_tmp = expr_asmgen_llvm(st->expr);
            ssa_count_tmp2 = expr_asmgen_llvm(st->next_expr);
            
            printf("    %%%d = call i32 (i8*, ...) @printf(i8* %%%d, i32 %%%d)\n",
                            ssa_count++, ssa_count_tmp, ssa_count_tmp2);
            
            break;
        default:
              break;  
    }

    stmt_asmgen_llvm(st->next, NULL);
}

int expr_asmgen_llvm(struct expr* target_expr){
    if(target_expr == NULL) return -1;
    int result = -1; 
    int left_ssa, right_ssa; 
    struct expr* tmp_expr;

    switch(target_expr->kind){
        case EXPR_NAME:
            symbol_asmgen_llvm(target_expr->symbol);
            result = ssa_count -1;
            
            break;
        case EXPR_CHAR:
        case EXPR_NUMBER:
            printf("    store i32 %d, i32* %%%d, align 4\n", target_expr->integer_value,
                            function_arg_count+1);
            printf("    %%%d = load i32, i32* %%%d, align 4\n",ssa_count++, function_arg_count+1);
            result = ssa_count - 1;
            break;

        case EXPR_STRING:
            break;
        
        case EXPR_MOD:
            left_ssa = expr_asmgen_llvm(target_expr->left);
            right_ssa = expr_asmgen_llvm(target_expr->right);
            printf("    %%%d = urem i32 %%%d, %%%d\n", ssa_count++, left_ssa, right_ssa);
            result = ssa_count - 1;
            break;
        case EXPR_DIV:
            left_ssa = expr_asmgen_llvm(target_expr->left);
            right_ssa = expr_asmgen_llvm(target_expr->right);
            printf("    %%%d = sdiv nsw i32 %%%d, %%%d\n", ssa_count++, left_ssa, right_ssa);
            result = ssa_count - 1;
            break;

        case EXPR_MUL:
            left_ssa = expr_asmgen_llvm(target_expr->left);
            right_ssa = expr_asmgen_llvm(target_expr->right);
            printf("    %%%d = mul nsw i32 %%%d, %%%d\n", ssa_count++, left_ssa, right_ssa);
            result = ssa_count - 1;
            break;
            
        case EXPR_MINUS:
            left_ssa = expr_asmgen_llvm(target_expr->left);
            right_ssa = expr_asmgen_llvm(target_expr->right);
            printf("    %%%d = sub nsw i32 %%%d, %%%d\n", ssa_count++, left_ssa, right_ssa);
            result = ssa_count - 1;

            break;
        case EXPR_ADD:
            left_ssa = expr_asmgen_llvm(target_expr->left);
            right_ssa = expr_asmgen_llvm(target_expr->right);
            printf("    %%%d = add nsw i32 %%%d, %%%d\n", ssa_count++, left_ssa, right_ssa);
            result = ssa_count - 1;
            break;
        case EXPR_ASSIGN:
            right_ssa = expr_asmgen_llvm(target_expr->right);
            if(target_expr->left && target_expr->left->kind == EXPR_NAME && 
                            target_expr->left->symbol->kind == SYMBOL_GLOBAL){
                printf("    store i32 %%%d, i32* @%s, align 4\n", 
                        right_ssa, target_expr->left->symbol->name);
            }else{
                printf("    store i32 %%%d, i32* %%%d, align 4\n", 
                            right_ssa, target_expr->left->symbol->which+function_arg_count+1);
            }
            break;

        case EXPR_ARG:
            if(target_expr->left && target_expr->left->kind != EXPR_NUMBER){
                target_expr->left->reg = expr_asmgen_llvm(target_expr->left); 
            }
        
            expr_asmgen_llvm(target_expr->right); 
            break;
        case EXPR_CALL:
            expr_asmgen_llvm(target_expr->right);
            printf("    %%%d = call i32 @%s", ssa_count++, target_expr->left->name);
            printf("(");
            tmp_expr = target_expr->right;
            while(tmp_expr){
                if(tmp_expr->left && tmp_expr->left->kind == EXPR_NUMBER){
                    printf("i32 %d", tmp_expr->left->integer_value);
                }else{
                    printf("i32 %%%d", tmp_expr->left->reg);
                }
                if(tmp_expr->right){
                    printf(", ");
                }
                tmp_expr = tmp_expr->right;
            }
            printf(")\n");
            result = ssa_count - 1;
            break;
        
        case EXPR_NOTEQUAL:
            left_ssa = expr_asmgen_llvm(target_expr->left);
            right_ssa = expr_asmgen_llvm(target_expr->right);
            printf("    %%%d = icmp ne i32  %%%d, %%%d\n", ssa_count++, left_ssa, right_ssa);
            result = ssa_count - 1;
            break;
        case EXPR_EQUAL:
            left_ssa = expr_asmgen_llvm(target_expr->left);
            right_ssa = expr_asmgen_llvm(target_expr->right);
            printf("    %%%d = icmp eq i32  %%%d, %%%d\n", ssa_count++, left_ssa, right_ssa);
            result = ssa_count - 1;
            break;
        case EXPR_LESS:
            left_ssa = expr_asmgen_llvm(target_expr->left);
            right_ssa = expr_asmgen_llvm(target_expr->right);
            printf("    %%%d = icmp ult i32  %%%d, %%%d\n", ssa_count++, left_ssa, right_ssa);
            result = ssa_count - 1;
            break;
        case EXPR_LESSEQU:
            left_ssa = expr_asmgen_llvm(target_expr->left);
            right_ssa = expr_asmgen_llvm(target_expr->right);
            printf("    %%%d = icmp ule i32  %%%d, %%%d\n", ssa_count++, left_ssa, right_ssa);
            result = ssa_count - 1;
            break;
        case EXPR_GREATER:
            left_ssa = expr_asmgen_llvm(target_expr->left);
            right_ssa = expr_asmgen_llvm(target_expr->right);
            printf("    %%%d = icmp ugt i32  %%%d, %%%d\n", ssa_count++, left_ssa, right_ssa);
            result = ssa_count - 1;
            break;
        case EXPR_GREATEREQU:
            left_ssa = expr_asmgen_llvm(target_expr->left);
            right_ssa = expr_asmgen_llvm(target_expr->right);
            printf("    %%%d = icmp uge i32  %%%d, %%%d\n", ssa_count++, left_ssa, right_ssa);
            result = ssa_count - 1;
            break;
        case EXPR_INC:
            left_ssa = expr_asmgen_llvm(target_expr->left);
            printf("    %%%d = add nsw i32 %%%d, 1\n",ssa_count++, left_ssa);
            printf("    store i32 %%%d, i32* %%%d, align 4\n", ssa_count - 1, 
                            target_expr->left->symbol->which+function_arg_count+1);
            result = ssa_count - 1;
            break;
        default:
            break;
    }

    return result;
}



int stmt_reg_count(struct stmt* st){
    if(st == NULL) return 0;
    int result = 0;
    switch(st->kind){
        case STMT_DECL:
            break;
        case STMT_EXPR:
            result = expr_reg_count(st->expr);
            break;
        case STMT_BLOCK:
            result = stmt_reg_count(st->body);
            break;
        case STMT_FOR:
        case STMT_IF_ELSE:
        case STMT_WHILE:
            break;
        case STMT_RETURN:
            result = expr_reg_count(st->expr);
            break;
        case STMT_PRINT:
            result = expr_reg_count(st->expr) + expr_reg_count(st->next_expr) + 1;
            break;
        default:
                break;
    }
    
    return result + stmt_reg_count(st->next);
}

int expr_reg_count(struct expr* target_expr){
    if(target_expr == NULL) return 0;
    int result = 0;
    switch(target_expr->kind){
        case EXPR_NAME:
        case EXPR_NUMBER:
             result = 1;
             break;
        case EXPR_LESS:
        case EXPR_LESSEQU:
        case EXPR_GREATER:
        case EXPR_GREATEREQU:
        case EXPR_EQUAL:
        case EXPR_NOTEQUAL:
        case EXPR_ADD:
        case EXPR_DIV:
        case EXPR_MINUS:
        case EXPR_MOD:
        case EXPR_MUL:
            result = expr_reg_count(target_expr->left) + expr_reg_count(target_expr->right) + 1;
            break;
        case EXPR_ARG:
            result = expr_reg_count(target_expr->left) + expr_reg_count(target_expr->right);
            break;
        case EXPR_CALL:
            result = expr_reg_count(target_expr->right) + 1;
            break;
        case EXPR_ASSIGN:
            result = expr_reg_count(target_expr->right);
            break;
        case EXPR_INC:
        case EXPR_DEC:
            result = 2;
            break;
        default:
            break;
    }
    return result;
}

