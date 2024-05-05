%{
#include <stdio.h>
#include <ast/ast.h>
void yyerror(const char *s);
extern int yylex(void);
extern NodePtr root;
%}

/// types
%union {
    int ival;
    char *ident;
    ExprPtr expr;
    OpType op;
}

%token <ival> INTCONST
%token <ident> IDENT
%token ADD SUB MUL DIV ASSIGN GT GE LT LE EQ NEQ MOD AND OR NOT 
%token IF ELSE WHILE BREAK RETURN TRUE FALSE VOID INT
%token LPAREN RPAREN LBRACKET RBRACKET LBRACE RBRACE COMMA SEMICOLON

%start CompUnit

%type <expr> CompUnit Decl BType VarDecl VarDefList VarDef ArrayIndexList InitVal FuncType
%type <expr> InitValList FuncDef FuncFParams FuncFParam NumList Block BlockItemList BlockItem
%type <expr> Stmt Exp LVal LValExpList PrimaryExp Number UnaryExp UnaryOp FuncRParams
%type <expr> MulExp AddExp RelExp EqExp LOrExp LAndExp

%left AND OR
%left ASSIGN GT GE LT LE EQ NEQ
%left ADD SUB 
%left MUL DIV MOD
%left NOT

%%
CompUnit    : Decl                                      { root->children->push_back($1); }
            | FuncDef                                   { root->children->push_back($1); }
            | CompUnit Decl                             { root->children->push_back($2); }
            | CompUnit FuncDef                          { root->children->push_back($2); }
            ;

Decl        : VarDecl                                   { $$ = $1; }
            ;

BType       : INT                                       { $$ = "int"; }
            ;

VarDecl     : BType VarDefList SEMICOLON                    { $$ = new VarDecl($1,$2); }
            ;

VarDefList  : VarDefList COMMA VarDef                           { $$ = $1; $$->children->push_back($3); }
            | VarDef                                            { $$ = new VarDefList(); $$->children->push_back($1);}
            ;

VarDef      : IDENT                                             { $$ = new VarDef(); }
            | IDENT ASSIGN InitVal                              { $$ = new VarDef(); }
            | IDENT ArrayIndexList                              { $$ = new VarDef(); }
            | IDENT ArrayIndexList ASSIGN InitVal               { $$ = new VarDef(); }
            ;

ArrayIndexList  : ArrayIndexList LBRACKET INTCONST RBRACKET         { $$ = $1; $$->children->push_back($3); }
                | LBRACKET INTCONST RBRACKET                        { $ = new ArrayIndexList(); $$->children->push_back($2); }
                ;






InitVal     : Exp                                                   { $$ = $1; }
            | LBRACE RBRACE                                         { $$ = new AST("test"); }
            | LBRACE InitValList RBRACE                             { $$ = new AST("test"); $$->insert($2); }
            ;

InitValList : InitValList COMMA InitVal                         { $$ = $1; $$->insert($3); }
            | InitVal                                           { $$ = new AST("test"); $$->insert($1); }
            ;







FuncType    : INT                                       { $$ = "int"; }
            | VOID                                      { $$ = "void"; }
            ;

FuncDef     : FuncType IDENT LPAREN RPAREN Block                       { $$ = new AST("FuncDef"); $$->dtype = $1->tokentype; $$->ID = $2; $$->insert($5); }
            | FuncType IDENT LPAREN FuncFParams RPAREN Block           { $$ = new AST("FuncDef"); $$->dtype = $1->tokentype; $$->ID = $2; $$->child = $4->child; $$->insert($6); }
            ;

FuncFParams : FuncFParams COMMA FuncFParam                  { $$ = $1; $$->insert($3); }
            | FuncFParam                                    { $$ = new AST("temp"); $$->insert($1); }
            ;

FuncFParam  : INT IDENT                                   { $$ = new AST("FuncFParam"); $$->ID = $2; }
            | INT IDENT NumList                           { $$ = new AST("FuncFParam"); $$->ID = $2; $$->insert($3); }
            ;

NumList     : NumList LBRACKET INTCONST RBRACKET        { $$ = $1; $$->insert(new AST("IntConst", $3)); }
            | LBRACKET RBRACKET                         { $$ = new AST("test"); }
            ;








Block           : LBRACE RBRACE                         { $$ = new AST("Block"); }
                | LBRACE BlockItemList RBRACE           { $$ = new AST("Block"); $$->child = $2->child; }
                ;

BlockItemList   : BlockItemList BlockItem               { $$ = $1; $$->insert($2); }
                | BlockItem                             { $$ = new AST("temp"); $$->insert($1); }
                ;

BlockItem       : Decl                                  { $$ = $1; }
                | Stmt                                  { $$ = $1; }
                ;

Stmt            : LVal ASSIGN Exp SEMICOLON                     { $$ = new AST("AssignStmt"); $$->insert($1); $$->insert($3); }
                | Exp SEMICOLON                                 { $$ = $1; $$->tokentype = "ExpStmt"; }
                | Block                                         { $$ = $1; $$->tokentype = "Block"; }
                | IF LPAREN Exp RPAREN Stmt ELSE Stmt           { $$ = new AST("IfStmt"); $$->insert($3); $$->insert($5); $$->insert($7); }
                | IF LPAREN Exp RPAREN Stmt                     { $$ = new AST("IfStmt"); $$->insert($3); $$->insert($5); }
                | WHILE LPAREN Exp RPAREN Stmt                  { $$ = new AST("WhileStmt"); $$->insert($3);$$->insert($5); }
                | RETURN SEMICOLON                              { $$ = new AST("ReturnStmt"); }
                | RETURN Exp SEMICOLON                          { $$ = new AST("ReturnStmt"); $$->insert($2); }
                ;









Exp         : LOrExp                                { $$ = $1; }
            ;

LVal        : IDENT                                 { $$ = new Identifier($1); $$->ID = $1; }
            | IDENT LValExpList                     { $$ = new Identifier($1); $$->ID = $1; $$->child = $2->child; }
            ;

LValExpList : LValExpList LBRACKET Exp RBRACKET     { $$ = $1; $$->insert($3); }
            | LBRACKET Exp RBRACKET                 { $$ = new AST("test"); $$->insert($2); }
            ;

PrimaryExp  : LPAREN Exp RPAREN                     { $$ = new PrimaryExp(); }
            | LVal                                  { $$ = new PrimaryExp(); }
            | Number                                { $$ = new PrimaryExp(); }
            ;

Number      : INTCONST                              { $$ = new IntegerLiteral($1); }
            ;

UnaryExp    : PrimaryExp                            { $$ = $1; }
            | IDENT LPAREN RPAREN                   { $$ = new UnaryExp(); }
            | IDENT LPAREN FuncRParams RPAREN       { $$ = new UnaryExp(); }
            | UnaryOp UnaryExp                      { $$ = new UnaryExp(); }
            ;

UnaryOp     : ADD                                   { $$ = "+"; }
            | SUB                                   { $$ = "-"; }
            | NOT                                   { $$ = "!"; }
            ;

FuncRParams : FuncRParams COMMA Exp                  { $$ = $1; $$->insert($3); }
            | Exp                                    { $$ = new AST("temp"); $$->insert($1); }
            ;







MulExp      : UnaryExp                              { $$ = $1; }
            | MulExp MUL UnaryExp                   { $$ = new BinaryExp(OpType::OP_Mul, $1, $3); }
            | MulExp DIV UnaryExp                   { $$ = new BinaryExp(OpType::OP_Div, $1, $3); }
            | MulExp MOD UnaryExp                   { $$ = new BinaryExp(OpType::OP_Mod, $1, $3); }
            ;

AddExp      : MulExp                                { $$ = $1; }
            | AddExp ADD MulExp                     { $$ = new BinaryExp(OpType::OP_Mul, $1, $3); }
            | AddExp SUB MulExp                     { $$ = new BinaryExp(OpType::OP_Mul, $1, $3); }
            ;







RelExp      : AddExp                                { $$ = $1; }
            | RelExp GT AddExp                      { $$ = new BinaryExp(OpType::OP_Gt, $1, $3); }
            | RelExp GE AddExp                      { $$ = new BinaryExp(OpType::OP_Ge, $1, $3); }
            | RelExp LT AddExp                      { $$ = new BinaryExp(OpType::OP_Lt, $1, $3); }
            | RelExp LE AddExp                      { $$ = new BinaryExp(OpType::OP_Le, $1, $3); }
            ;

EqExp       : RelExp                                { $$ = $1; }
            | EqExp EQ RelExp                       { $$ = new BinaryExp(OpType::OP_Eq, $1, $3); }
            | EqExp NEQ RelExp                      { $$ = new BinaryExp(OpType::OP_Ne, $1, $3); }
            ;

LAndExp     : EqExp                                 { $$ = $1; }
            | LAndExp AND EqExp                     { $$ = new BinaryExp(OpType::OP_Land, $1, $3); }
            ;

LOrExp      : LAndExp                               { $$ = $1; }
            | LOrExp OR LAndExp                     { $$ = new BinaryExp(OpType::OP_Lor, $1, $3); }
            ;

%%

void yyerror(const char *s) {
    printf("error: %s\n", s);
}
