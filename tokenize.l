%{
#include "ast.h"
#include "grammars.tab.h"

%}

%option noyywrap yylineno

%x COMMENT
%x IFILE

%%

"+"         { return TOKEN_ADD; }
"-"         { return TOKEN_MINUS; }
"*"         { return TOKEN_MUL; }
"/"         { return TOKEN_DIV; }
"%"         { return TOKEN_MOD; }
":"         { return TOKEN_COLON; }
";"         { return TOKEN_SEMI; }
"="         { return TOKEN_ASSIGN; }
"("         { return TOKEN_LPAREN; }
")"         { return TOKEN_RPAREN; }
"{"         { return TOKEN_LBRACE; }
"}"         { return TOKEN_RBRACE; }
","         { return TOKEN_COMMA;}
"=="        {return TOKEN_EQUAL;}
"!="        {return TOKEN_NOTEQUAL;}
">"         {return TOKEN_GREATER;}
"<"         {return TOKEN_LESS;}
"++"        {return TOKEN_INC;}
"--"        {return TOKEN_DEC;}
"["         {return TOKEN_LSQB;}
"]"         {return TOKEN_RSQB;}


"integer"   { return TOKEN_INTEGER; }
"void"      { return TOKEN_VOID; }
"boolean"   { return TOKEN_BOOL;}
"char"      { return TOKEN_TYPE_CHAR;}
"string"    { return TOKEN_TYPE_STRING;}
"array"     { return TOKEN_ARRAY;}
"function"  { return TOKEN_FUNC;}
"return"    { return TOKEN_RETURN; }
"print"      { return TOKEN_PRINT; }
"if"        { return TOKEN_IF; }
"else"      { return TOKEN_ELSE;}
"for"        { return TOKEN_FOR; }
"while"        { return TOKEN_WHILE; }
"true"      {return TOKEN_BOOL_TRUE;}
"false"     {return TOKEN_BOOL_FALSE;}

"//".*\n
"/*"        {BEGIN(COMMENT);}
<COMMENT>"*/"   {BEGIN(INITIAL);}
<COMMENT>([^*]|\n)+|.
<COMMENT><<EOF>>    {return 0;}

\'.\'      {yylval.name = (char*)((int)yytext[1]);return TOKEN_NUMBER;}
\".*\"      {yylval.name = strdup(yytext); return TOKEN_STRING;}

0[0-7]* |
0[Xx][0-9a-fA-F]+ |
[1-9][0-9]*      { yylval.name = (char*)atoi(yytext); return TOKEN_NUMBER;}

[_A-Za-z][_A-Za-z0-9]*  {yylval.name = strdup(yytext); return TOKEN_NAME;}
[ \t\n]+       {}
.           { printf("Invaild Token: %c\n", *yytext);}

%%
