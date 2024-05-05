%option noinput
%option nounput
%option noyywrap

%{
#include <ast/ast.h>
#include "sysy.tab.hh"
%}

digit [0-9]
blank [ \t\n]

%%

{digit}+        { yylval.ival = atoi(yytext); return INTCONST; }
[_a-zA-Z][_a-zA-Z0-9]*                { yylval.ident = strdup(yytext);  return IDENT; }
"+"             { return ADD; }
"-"             { return SUB; }
"*"             { return MUL; }
"/"             { return DIV; }
","             { return COMMA; }
";"             { return SEMICOLON; }
"("             { return LPAREN; }
")"             { return RPAREN; }
"["             { return LBRACKET;}
"]"             { return RBRACKET;}
"{"             { return LBRACE; }
"}"             { return RBRACE; }
"="             { return ASSIGN; }
">"             { return GT; }
">="            { return GE; }
"<"             { return LT; }
"<="            { return LE; }
"=="            { return EQ; }
"!="            { return NEQ; }
"%"             { return MOD; }
"&&"            { return AND; }
"||"            { return OR; }
"!"             { return NOT; }
if              { return IF; }
else            { return ELSE; }
while           { return WHILE; }
break           { return BREAK; }
return          { return RETURN; }
true            { return TRUE; }
false           { return FALSE; }
int             { return INT; }
void            { return VOID; }
\/\/[^\n]*      { }
\/\*([^*]|(\*+[^*/]))*\*\/        { }
{blank}         { }
.               { printf("ERROR(%s)\n", yytext); }

%%
