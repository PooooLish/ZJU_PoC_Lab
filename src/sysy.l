%option noinput
%option nounput
%option noyywrap

%{
#include <ast/ast.h>
#include "sysy.tab.hh"
#define TESTPRINT 0
%}

digit [0-9]
blank [ \t\n]

%%
","             { if (TESTPRINT) { printf("COMMA "); } return COMMA; }
";"             { if (TESTPRINT) { printf("SEMICOLON "); } return SEMICOLON; }
"("             { if (TESTPRINT) { printf("LPAREN "); } return LPAREN; }
")"             { if (TESTPRINT) { printf("RPAREN "); } return RPAREN; }
"["             { if (TESTPRINT) { printf("LBRACKET "); } return LBRACKET;}
"]"             { if (TESTPRINT) { printf("RBRACKET "); } return RBRACKET;}
"{"             { if (TESTPRINT) { printf("LBRACE "); } return LBRACE; }
"}"             { if (TESTPRINT) { printf("RBRACE "); } return RBRACE; }
"="             { if (TESTPRINT) { printf("ASSIGN "); } return ASSIGN; }
">"             { if (TESTPRINT) { printf("GT "); } return GT; }
">="            { if (TESTPRINT) { printf("GE "); } return GE; }
"<"             { if (TESTPRINT) { printf("LT "); } return LT; }
"<="            { if (TESTPRINT) { printf("LE "); } return LE; }
"=="            { if (TESTPRINT) { printf("EQ "); } return EQ; }
"!="            { if (TESTPRINT) { printf("NEQ "); } return NEQ; }
"+"             { if (TESTPRINT) { printf("ADD "); } return ADD; }
"-"             { if (TESTPRINT) { printf("SUB "); } return SUB; }
"*"             { if (TESTPRINT) { printf("MUL "); } return MUL; }
"/"             { if (TESTPRINT) { printf("DIV "); } return DIV; }
"%"             { if (TESTPRINT) { printf("MOD "); } return MOD; }
"&&"            { if (TESTPRINT) { printf("AND "); } return AND; }
"||"            { if (TESTPRINT) { printf("OR "); } return OR; }
"!"             { if (TESTPRINT) { printf("NOT "); } return NOT; }
if              { if (TESTPRINT) { printf("IF "); } return IF; }
else            { if (TESTPRINT) { printf("ELSE "); } return ELSE; }
while           { if (TESTPRINT) { printf("WHILE "); } return WHILE; }
break           { if (TESTPRINT) { printf("BREAK "); } return BREAK; }
return          { if (TESTPRINT) { printf("RETURN "); } return RETURN; }
true            { if (TESTPRINT) { printf("TRUE "); } return TRUE; }
false           { if (TESTPRINT) { printf("FALSE "); } return FALSE; }
int             { if (TESTPRINT) { printf("INT "); } return INT; }
void            { if (TESTPRINT) { printf("VOID "); } return VOID; }
{blank}         { if (TESTPRINT) { printf("BLANK "); }; }
\/\/[^\n]*      { if (TESTPRINT) { printf("SINGLE_LINE_COMMENTS "); }; }
\/\*([^*]|(\*+[^*/]))*\*\/                                      { if (TESTPRINT) { printf("MULTI_LINE_COMMENTS "); }; }
[_a-zA-Z][_a-zA-Z0-9]*                { if (TESTPRINT) { printf("IDENT "); } yylval.ident = strdup(yytext);  return IDENT; }
{digit}+        { if (TESTPRINT) { printf("INTCONST "); }  yylval.ival = atoi(yytext); return INTCONST; }
.               {}

%%
