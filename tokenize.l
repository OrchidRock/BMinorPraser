%{
#include "ast.h"
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
"["         {return TOKEN_LSQB;}
"]"         {return TOKEN_RSQB;}


"integer"   { return TOKEN_INTEGER; }
"void"      { return TOKEN_VOID; }
"boolean"   { return TOKEN_BOOL;}
"char"      { return TOKEN_CHAR;}
"string"    { return TOKEN_TYPE_STRING;}
"array"     { return TOKEN_ARRAY;}
"function"  { return TOKEN_FUNC;}
"return"    { return TOKEN_RETURN; }
"print"      { return TOKEN_PRINT; }
"if"        { return TOKEN_IF; }
"else"      { return TOKEN_ELSE;}
"for"        { return TOKEN_FOR; }

\".*\"      {yylval.name = yytext; return TOKEN_STRING;}
[0-9]+      { yylval.name = (char*)atoi(yytext); return TOKEN_NUMBER;}
[A-Za-z][0-9A-Za-z_]*  {yylval.name = strcopy(yytext); return TOKEN_NAME;}
[ \t]+       {}
.           { printf("Invaild Token: %c\n", *yytext);}

%%
