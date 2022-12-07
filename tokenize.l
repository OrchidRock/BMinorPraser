%{
#include "grammars.tab.h"
%}

%%

"+"         { return TOKEN_ADD; }
"-"         { return TOKEN_MINUS; }
"*"         { return TOKEN_MUL; }
"/"         { return TOKEN_DIV; }
":"         { return TOKEN_COLON; }
";"         { return TOKEN_SEMI; }
"="         { return TOKEN_ASSIGN; }
"("         { return TOKEN_LPAREN; }
")"         { return TOKEN_RPAREN; }
"{"         { return TOKEN_LBRACE; }
"}"         { return TOKEN_RBRACE; }
","         { return TOKEN_COMMA;}
"=="        {return TOKEN_EQUAL;}
">"         {return TOKEN_GREATER;}
"<"         {return TOKEN_LESS;}
"++"        {return TOKEN_INC;}
"--"        {return TOKEN_DEC;}


"integer"   { return TOKEN_INTEGER; }
"function"  { return TOKEN_FUNC; }
"return"    { return TOKEN_RETURN; }
"void"      { return TOKEN_VOID; }
"print"      { return TOKEN_PRINT; }
"if"        { return TOKEN_IF; }
"for"        { return TOKEN_FOR; }

[0-9]+      { yylval = atoi(yytext);return TOKEN_NUMBER;}
[A-Za-z][0-9A-Za-z_]*  {return TOKEN_NAME;}
[ \t]       {}
.           { printf("Invaild Token: %c\n", *yytext);}

%%
