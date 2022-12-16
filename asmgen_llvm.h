
#ifndef __ASMGEN_LLVM_H__
#define __ASMGEN_LLVM_H__


#include "ast.h"


int expr_asmgen_llvm(struct expr* exp);
void decl_asmgen_llvm(struct decl*);
void stmt_asmgen_llvm(struct stmt*, struct type*);

#endif
