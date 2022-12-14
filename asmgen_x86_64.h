#ifndef __ASMGEN_X86_64_H__
#define __ASMGEN_X86_64_H__


#include "ast.h"


void expr_asmgen(struct expr* exp);
void decl_asmgen(struct decl*);
void stmt_asmgen(struct stmt*, struct type*);

#endif
